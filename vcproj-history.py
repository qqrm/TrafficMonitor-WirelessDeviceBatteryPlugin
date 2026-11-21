p = "/home/qqrm/wb-plugin-ui/WirelessDeviceBattery.vcxproj"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace('<ClCompile Include="OptionsDlg.cpp" />',
              '<ClCompile Include="BatteryHistory.cpp" />\n    <ClCompile Include="OptionsDlg.cpp" />')
s = s.replace('<ClInclude Include="DeviceBrandMasks.h" />',
              '<ClInclude Include="BatteryHistory.h" />\n    <ClInclude Include="DeviceBrandMasks.h" />')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("vcproj history")
