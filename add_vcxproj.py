p = "/home/qqrm/wb-plugin-ui/WirelessDeviceBattery.vcxproj"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace('<ClInclude Include="framework.h" />',
              '<ClInclude Include="DeviceBrandMasks.h" />\n    <ClInclude Include="framework.h" />\n    <ClInclude Include="LogitechMark.h" />')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("vcxproj patched")
