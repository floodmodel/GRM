<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis version="2.18.13" minimumScale="inf" maximumScale="1e+08" hasScaleBasedVisibilityFlag="0">
  <pipe>
    <rasterrenderer opacity="1" alphaBand="-1" classificationMax="700" classificationMinMaxOrigin="MinMaxFullExtentExact" band="1" classificationMin="100" type="singlebandpseudocolor">
      <rasterTransparency/>
      <rastershader>
        <colorrampshader colorRampType="INTERPOLATED" clip="0">
          <item alpha="255" value="100" label="시가화/건조지역" color="#ccccce"/>
          <item alpha="255" value="200" label="농업지역" color="#b48ec3"/>
          <item alpha="255" value="300" label="산림지역" color="#85cb8b"/>
          <item alpha="255" value="400" label="초지" color="#d7ecd5"/>
          <item alpha="255" value="500" label="습지" color="#008837"/>
          <item alpha="255" value="700" label="수역" color="#1c37e7"/>
        </colorrampshader>
      </rastershader>
    </rasterrenderer>
    <brightnesscontrast brightness="0" contrast="0"/>
    <huesaturation colorizeGreen="128" colorizeOn="0" colorizeRed="255" colorizeBlue="128" grayscaleMode="0" saturation="0" colorizeStrength="100"/>
    <rasterresampler maxOversampling="2"/>
  </pipe>
  <blendMode>0</blendMode>
</qgis>
