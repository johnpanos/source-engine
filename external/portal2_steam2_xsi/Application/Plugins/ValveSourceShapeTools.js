////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Softimage.
// ValveSource Shape Tools (for XSI Version 4.2)
//
////////////////////////////////////////////////////////////////
var gToolName = "ValveSource Shape Tools";
var gToolPath = "";
var gDebug = false;

function XSILoadPlugin( in_reg )
{
	// init tool path	
	var fso = XSIFactory.CreateActiveXObject("Scripting.FileSystemObject");
	var strPluginsPath = in_reg.OriginPath;
	gToolPath = fso.GetParentFolderName(fso.GetParentFolderName(strPluginsPath)) + "\\";

	in_reg.Author = "ValveSource and Softimage" ;
	in_reg.Name = "ValveSource Shape Tools" ;
	in_reg.Major = 1 ;
	in_reg.Minor = 1 ;
	in_reg.Help = gToolPath + "Doc\\" + gToolName.replace(/\s*/g, "") + ".htm"; ;
	in_reg.URL = "www.softimage.com";

	in_reg.RegisterCommand( "vstShowShapeToolbar","vstShowShapeToolbar" );
	in_reg.RegisterCommand( "vstCopyShape","vstCopyShape" );
	in_reg.RegisterCommand( "vstSetCopyShapeCluster","vstSetCopyShapeCluster" );

	return true ;
}

////////////////////////////////////////////////////////////////
// Command: vstShowShapeToolbar
////////////////////////////////////////////////////////////////

function vstShowShapeToolbar_Execute()
{
	OpenView("ValveSource Shape Toolbar");
}

////////////////////////////////////////////////////////////////
// Command: vstCopyShapeKey
//
// Copy the shape for the selected vertices on the active cluster from
// one frame to another. The result is weighted between the shape for
// the source and destination frames based on the weight map created
// from the proportional modeling.
//
////////////////////////////////////////////////////////////////
function vstCopyShapeKey_Init( ctxt )
{
	var oCmd = ctxt.Source;
	oCmd.Description = "Copy the shape for the selected vertices from one frame to another";
}

function vstCopyShapeKey_Execute()
{
  // Get the source and target frames.

  var playControl = Application.ActiveProject.Properties("Play Control");
  var srcFrame = playControl.Parameters("Current").Value;
  var destFrame = playControl.Parameters("Key").Value;

  LogMessage("vstCopyShape: source frame = " + srcFrame + ", dest frame = " + destFrame);

  // Get the name of the cluster.

	LogMessage(selection(0).obj.fullName);
  var obj = GetValue(selection(0).obj).parent;

	LogMessage(obj.fullName);

  var prop = obj.Properties("vstCopyShapeCluster");
  if (!prop) {
    LogMessage("model does not have vstCopyShapeCluster property");
    return;
  }

  var clusterName = prop.text.value;
  LogMessage("vstCopyShape: cluster name is " + clusterName);

  // Find the actual cluster.

  var srcGeom = obj.activePrimitive.geometry(srcFrame);

  var cluster = srcGeom.clusters(clusterName);
  if (!cluster) {
    LogMessage("cannot find cluster: " + clusterName);
    return;
  }

  // Translate the selection a bit to create the operator and weight map.

  Translate(null, 0, 0, 0, siRelative, siGlobal, siObj, siXYZ,
	    null, null, null, null, null, null, null, null, null, 0);

  // Find the operator responsible for the translation.

  var op = null;
  var chenum = new Enumerator(obj.activePrimitive.constructionHistory);
  for (; !chenum.atEnd(); chenum.moveNext()) {
    if (chenum.item().type == "movecomponentop") {
      op = chenum.item();
      break;
    }
  }
  
  if (op == null) {
    LogMessage("cannot find MoveComponent operator?");
    return;
  }

  // Dump out the operator's ports. This was useful to analyze where
  // the various operator inputs were connected.

  /*
  LogMessage("op is " + op.fullName);

  var npg = op.GetNumPortGroups();
  LogMessage("num groups is " + npg);

  for (var g = 0; g < npg; ++g) {
    LogMessage("group " + g);
    var np = op.GetNumPortsInGroup(g);
    LogMessage("  num ports is " + op.GetNumPortsInGroup(g));

    for (var p = 0; p < np; ++p) {
      var port = op.portAt(p, g, 0);
      LogMessage("  port " + p + " is " + ClassName(port) + " -> " + port.target2.fullName);
    }
  }
  */

  // Grab the weight values for the points. Port indices were
  // empirically found using the logging code above.

  var wmap = op.portAt(2, 1, 0).target2;

  var warray = (new VBArray(wmap.Elements.Array)).toArray();

  var cls = op.portAt(0, 1, 0).target2; 
  var sel = cls.CreateSubComponent();

  // Find the shape key at the source frame.

  var srcKey = ActiveShapeKeyAtFrame(clusterName, obj, srcFrame);
  if (!srcKey) {
    LogMessage("cannot find shape key at source frame index " + srcFrame);
    Undo();
    return;
  }

  var srcKeySources = srcKey.sourceItems;

  // Find the shape key at the destination frame; this is the key we'll change.

  var destKey = ActiveShapeKeyAtFrame(clusterName, obj, destFrame);
  if (!destKey) {
    LogMessage("cannot find shape key at target frame index " + destFrame);
    Undo();
    return;
  }

  var destKeySources = destKey.sourceItems;

  // Get an array of the position vectors driving the shape animation of the cluster.

  var srcKeySource = srcKeySources.Item(0).source;
  var sarray = srcKeySource.Elements.Array.toArray();

  var destKeySource = destKeySources.Item(0).source;
  var darray = destKeySource.Elements.Array.toArray();

  // Get the sub component for the cluster.

  var csc = cluster.CreateSubComponent();

  // For each selected component, if it is in the target cluster,
  // compute the blended position.

  var n = sel.ComponentCollection.Count;

  var pbar = XSIUIToolkit.ProgressBar;
  pbar.maximum = n;
  pbar.step = 1;
  pbar.visible = true;
  pbar.caption = "Blending shape keys";

  var out = new Array(darray.length);
  for (var i = 0; i < darray.length; ++i)
    out[i] = darray[i];

  for (var i = 0; i < n; ++i) {
    var p = sel.ComponentCollection.Item(i);

    var di = ComponentCollectionContainsIndex(csc.ComponentCollection, p.index);
    if (di >= 0) {
      var w = warray[i];

      var sx = sarray[di * 3];
      var sy = sarray[di * 3 + 1];
      var sz = sarray[di * 3 + 2];

      var dx = darray[di * 3];
      var dy = darray[di * 3 + 1];
      var dz = darray[di * 3 + 2];

      out[di * 3] = w * sx + (1 - w) * dx;
      out[di * 3 + 1] = w * sy + (1 - w) * dy;
      out[di * 3 + 2] = w * sz + (1 - w) * dz;
    }

    pbar.increment();
  }

  // Remove the operator. This has to happen before updating the shape
  // key (otherwise the undo will apply to the shape key) but after
  // we're done using the weight map associated with the operator.

  op.disconnect();

  // Stick the resultant values into the shape key.

  destKeySource.Elements.Array = out;
}

// Return the shape key that is active at a given frame. This assumes
// that the mixer contains a number of tracks, each of which contains
// a single clip, and that each clip's weight curve is non-zero at
// only a single frame.

function ActiveShapeKeyAtFrame(clusterName, obj, frame)
{
  // This is really pitifully annoying and slow...

  var mixer = obj.Model.Mixer;
  var tracks = mixer.Tracks;

  for (var i = 0; i < tracks.Count; ++i) {

    var track = tracks.Item(i);

    if (track.Clips.Count == 1) {

      var clip = track.Clips(0);
      if (clip.name.substring(0, clusterName.length) == clusterName) {
	// This is the compound clip containing the various shape
	// clips. It is a clip container.

	// Look for the track that contains one clip that has a weight
	// function that is nonzero at the specified frame.

	var clipTracks = clip.Tracks;
	for (var j = 0; j < clipTracks.Count; ++j) {

	  var track = clipTracks.Item(j);
	  if (track.Clips.Count == 1) {

	    var clip = track.Clips(0);
	    var weight = clip.Parameters("Weight");
	    if (weight) {
	      var w = weight.value(frame);
	      if (w > 0) {
		// This is the clip with the shape key we want.
		var src = clip.Source;
		LogMessage("vstCopyShape: clip at frame " + frame + " has source " + src);
		return src;
	      }
	    }
	  }
	}
      }
    }
  }

  return null;
}

// Returns true if a component collection includes the given index, or
// false otherwise.

function ComponentCollectionContainsIndex(coll, index)
{
  var n = coll.Count;
  for (var i = 0; i < n; ++i) {
    var p = coll.Item(i);
    if (p.index == index)
      return i;
  }

  return -1;
}

// Return the shape key that is active at a given frame. This assumes
// that the mixer contains a number of tracks, each of which contains
// a single clip, and that each clip's weight curve is non-zero at
// only a single frame.

function ActiveShapeKeyAtFrame(clusterName, obj, frame)
{
  // This is really pitifully annoying and slow...

  var mixer = obj.Model.Mixer;
  var tracks = mixer.Tracks;

  for (var i = 0; i < tracks.Count; ++i) {

    var track = tracks.Item(i);

    if (track.Clips.Count == 1) {

      var clip = track.Clips(0);
      if (clip.name.substring(0, clusterName.length) == clusterName) {
	// This is the compound clip containing the various shape
	// clips. It is a clip container.

	// Look for the track that contains one clip that has a weight
	// function that is nonzero at the specified frame.

	var clipTracks = clip.Tracks;
	for (var j = 0; j < clipTracks.Count; ++j) {

	  var track = clipTracks.Item(j);
	  if (track.Clips.Count == 1) {

	    var clip = track.Clips(0);
	    var weight = clip.Parameters("Weight");
	    if (weight) {
	      var w = weight.value(frame);
	      if (w > 0) {
		// This is the clip with the shape key we want.
		var src = clip.Source;
		LogMessage("vstCopyShape: clip at frame " + frame + " has source " + src);
		return src;
	      }
	    }
	  }
	}
      }
    }
  }

  return null;
}

// Return the shape key that is active at a given frame. This assumes
// that the mixer contains a number of tracks, each of which contains
// a single clip, and that each clip's weight curve is non-zero at
// only a single frame.

function ActiveShapeKeyAtFrame(clusterName, obj, frame)
{
  // This is really pitifully annoying and slow...

  var mixer = obj.Model.Mixer;
  var tracks = mixer.Tracks;

  for (var i = 0; i < tracks.Count; ++i) {

    var track = tracks.Item(i);

    if (track.Clips.Count == 1) {

      var clip = track.Clips(0);
      if (clip.name.substring(0, clusterName.length) == clusterName) {
	// This is the compound clip containing the various shape
	// clips. It is a clip container.

	// Look for the track that contains one clip that has a weight
	// function that is nonzero at the specified frame.

	var clipTracks = clip.Tracks;
	for (var j = 0; j < clipTracks.Count; ++j) {

	  var track = clipTracks.Item(j);
	  if (track.Clips.Count == 1) {

	    var clip = track.Clips(0);
	    var weight = clip.Parameters("Weight");
	    if (weight) {
	      var w = weight.value(frame);
	      if (w > 0) {
		// This is the clip with the shape key we want.
		var src = clip.Source;
		LogMessage("vstCopyShape: clip at frame " + frame + " has source " + src);
		return src;
	      }
	    }
	  }
	}
      }
    }
  }

  return null;
}

// Returns true if a component collection includes the given index, or
// false otherwise.

function ComponentCollectionContainsIndex(coll, index)
{
  var n = coll.Count;
  for (var i = 0; i < n; ++i) {
    var p = coll.Item(i);
    if (p.index == index)
      return i;
  }

  return -1;
}

// Returns true if a component collection includes the given index, or
// false otherwise.

function ComponentCollectionContainsIndex(coll, index)
{
  var n = coll.Count;
  for (var i = 0; i < n; ++i) {
    var p = coll.Item(i);
    if (p.index == index)
      return i;
  }

  return -1;
}

////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Softimage.
//
// command: vstCopyShape
//
// Copy the shape for the selected vertices on the active cluster from
// one frame to another. The result is weighted between the shape for
// the source and destination frames based on the weight map created
// from the proportional modeling.
//
// $Id: CopyShape.js,v 1.1.1.1 2004/07/16 23:40:38 cbecker Exp $
//
////////////////////////////////////////////////////////////////
function vstCopyShape_Init( ctxt )
{
	var oCmd = ctxt.Source;
	oCmd.Description = "Copy shape from select shape cluster";
	oCmd.ReturnValue = false;

	return true;
}

function vstCopyShape_Execute()
{
  // Get the source and target frames.

  var playControl = Application.ActiveProject.Properties("Play Control");
  var srcFrame = playControl.Parameters("Current").Value;
  var destFrame = playControl.Parameters("Key").Value;

  if ( ! selection.count ) 
  {
    LogMessage("vstCopyShape: please select a shape cluster",siError);
    return;
  }
  
  Trace("vstCopyShape: source frame = " + srcFrame + ", dest frame = " + destFrame);

  // Get the name of the cluster.
  var obj = GetValue(selection(0).obj).parent;

  var prop = obj.Properties("vstCopyShapeCluster");
  if (!prop) {
    LogMessage("vstCopyShape: model does not have vstCopyShapeCluster property",siError);
    return;
  }

  var clusterName = prop.text.value;
  Trace("vstCopyShape: cluster name is " + clusterName);

  // Find the actual cluster.

  var srcGeom = obj.activePrimitive.geometry(srcFrame);

  var cluster = srcGeom.clusters(clusterName);
  if (!cluster) {
    LogMessage("vstCopyShape: cannot find cluster: " + clusterName,siError);
    return;
  }

  // Translate the selection a bit to create the operator and weight map.

  Translate(null, 0, 0, 0, siRelative, siGlobal, siObj, siXYZ,
	    null, null, null, null, null, null, null, null, null, 0);

  // Find the operator responsible for the translation.

  var op = null;
  var chenum = new Enumerator(obj.activePrimitive.constructionHistory);
  for (; !chenum.atEnd(); chenum.moveNext()) {
    if (chenum.item().type == "movecomponentop") {
      op = chenum.item();
      break;
    }
  }
  
  if (op == null) {
    LogMessage("vstCopyShape: cannot find MoveComponent operator?",siError);
    return;
  }

  // Dump out the operator's ports. This is useful to analyze how
  // the various operator inputs are connected.

  /*
  LogMessage("op is " + op.fullName);

  var npg = op.GetNumPortGroups();
  LogMessage("num groups is " + npg);

  for (var g = 0; g < npg; ++g) {
    LogMessage("group " + g);
    var np = op.GetNumPortsInGroup(g);
    LogMessage("  num ports is " + op.GetNumPortsInGroup(g));

    for (var p = 0; p < np; ++p) {
      var port = op.portAt(p, g, 0);
      LogMessage("  port " + p + " is " + ClassName(port) + " -> " + port.target2.fullName);
    }
  }
  */

  // Grab the weight values for the points. Port indices were
  // empirically found using the logging code above.

  var wmap = op.portAt(2, 1, 0).target2;

  var warray = (new VBArray(wmap.Elements.Array)).toArray();

  var cls = op.portAt(0, 1, 0).target2; 
  var sel = cls.CreateSubComponent();

  // Find the shape key at the source frame.

  var srcKey = ActiveShapeKeyAtFrame(clusterName, obj, srcFrame);
  if (!srcKey) {
    LogMessage("vstCopyShape: cannot find shape key at source frame index " + srcFrame,siError);
    Undo();
    return;
  }

  var srcKeySources = srcKey.sourceItems;

  // Find the shape key at the destination frame; this is the key we'll change.

  var destKey = ActiveShapeKeyAtFrame(clusterName, obj, destFrame);
  if (!destKey) {
    LogMessage("vstCopyShape: cannot find shape key at target frame index " + destFrame,siError);
    Undo();
    return;
  }

  var destKeySources = destKey.sourceItems;

  // Get an array of the position vectors driving the shape animation of the cluster.

  var srcKeySource = srcKeySources.Item(0).source;
  var sarray = srcKeySource.Elements.Array.toArray();

  var destKeySource = destKeySources.Item(0).source;
  var darray = destKeySource.Elements.Array.toArray();

  // Get the sub component for the cluster.

  var csc = cluster.CreateSubComponent();

  // For each selected component, if it is in the target cluster,
  // compute the blended position.

  var n = sel.ComponentCollection.Count;

  var pbar = XSIUIToolkit.ProgressBar;
  pbar.maximum = n;
  pbar.step = 1;
  pbar.visible = true;
  pbar.caption = "Blending shape keys";

  var out = new Array(darray.length);
  for (var i = 0; i < darray.length; ++i)
    out[i] = darray[i];

  for (var i = 0; i < n; ++i) {
    var p = sel.ComponentCollection.Item(i);

    var di = ComponentCollectionContainsIndex(csc.ComponentCollection, p.index);
    if (di >= 0) {
      var w = warray[i];

      var sx = sarray[di * 3];
      var sy = sarray[di * 3 + 1];
      var sz = sarray[di * 3 + 2];

      var dx = darray[di * 3];
      var dy = darray[di * 3 + 1];
      var dz = darray[di * 3 + 2];

      out[di * 3] = w * sx + (1 - w) * dx;
      out[di * 3 + 1] = w * sy + (1 - w) * dy;
      out[di * 3 + 2] = w * sz + (1 - w) * dz;
    }

    pbar.increment();
  }

	pbar.visible = false;

  // Remove the operator. This has to happen before updating the shape
  // key (otherwise the undo will apply to the shape key) but after
  // we're done using the weight map associated with the operator.

  op.disconnect();

  // Stick the resultant values into the shape key.

  destKeySource.Elements.Array = out;
}

////////////////////////////////////////////////////////////////
//
// Command: vstSetCopyShapeCluster
//
// Given a selected cluster, this command sets a property called
// vstCopyShapeCluster on the cluster's geometry object. This property
// contains the name of the cluster for later use with the vstCopyShape
// command.
//
// Make sure the current selection is a cluster, and if it is, add a
// property to the model that saves the name of the cluster for
// subsequent shape copy operations.
//
// Copyright (c) 2004 Softimage.
//
// $Id: vstSetCopyShapeCluster.js,v 1.1.1.1 2004/07/16 23:40:38 cbecker Exp $
//
////////////////////////////////////////////////////////////////
function vstSetCopyShapeCluster_Init( ctxt )
{
	var oCmd = ctxt.Source;
	oCmd.Description = "Initialize polygon mesh vertex cluster for copyable shape cluster";
}

function vstSetCopyShapeCluster_Execute()
{
  var sel = selection(0);

  if ( sel == null )
  {
	LogMessage("select a polymesh first");
	return;
  }
	
  if (sel.type == siVertexCluster) {
    var obj = sel.parent.parent.parent;
    var prop = obj.Properties("vstCopyShapeCluster");
    if (!prop)
      prop = obj.AddProperty("Annotation", false, "vstCopyShapeCluster");
    prop.text = sel.name;
    LogMessage("copy shape cluster set to " + sel.name);
  }
  else
    LogMessage("selection is not a vertex cluster");
}

function Trace( str )
{
	if ( gDebug )
		Application.Logmessage(str);
}

