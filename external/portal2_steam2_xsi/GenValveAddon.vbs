dim strLocationOfAddons
dim strAddonFilename

strLocationOfAddons = "S:\ThirdParty\ClientTools\Addons\ValveSource"
strAddonFilename = "S:\ThirdParty\ClientTools\Addons\ValveSource\ValveSource.xsiaddon"

PackageAddon strLocationOfAddons, strAddonFilename, True, "src;Debug;Ship"
