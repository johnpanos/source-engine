HelperBone Tool v1.0 rev12
==========================

What's new with rev12
=====================
- fix the dialog that appears when trying to import a model asking you if you want to overwrite file.
- fix the rotation in the helperbone ( #90494 - CMatrix3::MulInPlace ) when child  & parent orientation doesn't change

What's new with rev11
=====================

- implement delete helper button
- implement delete helper trigger button
- port axis interpolator operator from vbscript to C++ ( source include under src\ directory )
- add set ori(entation) button for setting the trigger orientation from the current bone positions
- implement resetting the helper from data in the .vrd file on import 
- removed data tab from AxisInterpolatorTool property page
  The trigger data for each axisinterpop was stored on the property using IDs. It 
  has now been moved to the axisinterpop
- remove ID & Data parameters from axisinterop, these where used to get the trigger data from the
  AxisInterpolatorTool property. The triggers are now stored in a Triggers string parameter. 
- re-organized the HelperTool toolbar. There are 2 sections for the user: user commands & main (show,help,edit)

