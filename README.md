# GRM(Grid based Rainfall-Runoff Model)

The model indroduction and the download guidelines are wiki page at https://github.com/floodmodel/GRM/wiki

# Update history
2018.11.16. Convert VB .NET codes to C#  
2019.07.22. Revision in channel information to setup each most downstream basin  
2020.01.08. GRM2020 released  
2020.04.16. GRM2020 C++ version released  
2020.08.29. Bug fix in /f option  
2020.09.08. Bug fix in /a option  
2020.09.10. Bug fix in Flow control simulation  
2021.01.07. Bug fix in bitmap image output renderer  
2021.01.16. Bug fix in calculating average rainfall when using '/f' option  
2021.02.22. Bug fix in writing GRM version  
2021.04.19. Add an error check process for initial storages of reservoirs  
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