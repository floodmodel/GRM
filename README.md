# GRM(Grid based rainfall-Runoff Model)

The model indroduction and the download guidelines are wiki page at https://github.com/floodmodel/GRM/wiki
<br/><br/>

# Update history
2018.11.16. Convert VB .NET codes to C#  
2019.07.22. Revision in channel information to setup each most downstream basin  
2020.01.08. GRM2020 was released  
2020.04.16. GRM2020 C++ version was released  
2020.09.08. Bug fix in '/f' and '/a' option  
2020.09.22. Bug fix in Flow control simulation  
2021.01.07. Bug fix in bitmap image output renderer  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Bug fix in calculating average rainfall when using '/f' option  
2021.02.22. Add rainfall data writing process for each watch point.  
2021.04.19. Add an error check process for the initial storages of reservoirs.  
2021.04.23. GRM v2021.04 beta was released.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The GRM was improved to enable more detailed simulation for reservoir operation.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The manuals and sample data were updated.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;GRM Analyzer was uploaded.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The feature of printing reservoir inflow data was added.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Some bugs about calculating reservoir operation were fixed.  
2021.07. Minor revision in using initial stream flow value.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;gentle.dll file was embedded in GRMAnalyzer.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Add some error messages for reading flow direction file and setting most downstream catchment.  
2021.08. Add some error messages for setting flow network.  
2021.09. A critial bug fix in flow network analysis(about finding recursive flow).  
2021.10. A minor bug fix in counting gmp files progress percentage when using '/f' option.  
2021.11. A minor bug fix in using '/a' option for the gmp file that includes 'Inlet'. 
2022.02. Add some error messages.
2022.07. Revision in reading ASC raster file.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Bug fix in reduction process of OpenMP.        
2022.11. Bug fix in setting initial stream flow.   
2022.11. GRM v2022 beta was uploaded.        
2023.03. GRM v2022 was uploaded as the stable version.   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Interception, evapotranspiration, and snow melt modules were added for continuous simulation.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Input and output data were revised.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; In AutoROM, the maximum limitation of the reservoir outflow can be applied.  
2023.04. The flow in soil layers within the stream control volume was improved.   
2023.05. Bug fix for multiple flow control types in a control volume.   
2024.03. GRM v2024 was released.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; The GRM was improved to enable detension pond simulation.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Initial precipitation loss parameter was added.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; The manuals and sample data were updated.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Files for previous version were moved to DownloadOldVersion folder.  
2024.05. Constant ratio option was applied to the RigidROM.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Output value separator option was added.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Bug fix for numeric value includes comma.   
2024.08. Revision in FC module for maximum storage during flood season.  
2024.11. Bugs and errors in calculating evapotranspiration were revised.  
2025.02. GRM v2025 was released.   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Potential evapotranspiration methods were revised (the units, parameter estimation methods, etc.).   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; FAO Penman Monteith method was added.   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Potential and actual evapotranspiration raster outputs fuction were added.    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Cumulative bug fixed.





   