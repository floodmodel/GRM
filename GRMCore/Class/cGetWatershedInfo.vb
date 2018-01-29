Public Class cGetWatershedInfo
    Public grmPrj As New cProject
    Private mstreamFPN As String
    Private mlandCoverFPN As String
    Private msoilTextureFPN As String
    Private msoilDepthFPN As String
    Private miniSoilSaturationFPN As String
    Private miniChannelFlowFPN As String
    Sub New(watershedFPN As String,
                slopeFPN As String,
                fdirFPN As String,
                facFPN As String,
            Optional streamFPN As String = "",
            Optional landCoverFPN As String = "",
            Optional soilTextureFPN As String = "",
            Optional soilDepthFPN As String = "",
            Optional iniSoilSaturationFPN As String = "",
            Optional iniChannelFlowFPN As String = "")

        grmPrj.ReadLayerWSandSetBasicInfo(watershedFPN, True)
        grmPrj.ReadLayerSlope(slopeFPN, True)
        grmPrj.ReadLayerFdir(fdirFPN, True)
        grmPrj.ReadLayerFAcc(facFPN, True)
        If streamFPN <> "" AndAlso File.Exists(streamFPN) Then
            grmPrj.ReadLayerStream(streamFPN, True)
            mstreamFPN = streamFPN
        End If
        If landCoverFPN <> "" AndAlso File.Exists(landCoverFPN) Then
            grmPrj.ReadLandCoverFile(landCoverFPN, True)
            mlandCoverFPN = landCoverFPN
        End If
        If soilTextureFPN <> "" AndAlso File.Exists(soilTextureFPN) Then
            grmPrj.ReadSoilTextureFile(soilTextureFPN, True)
            msoilTextureFPN = soilTextureFPN
        End If
        If soilDepthFPN <> "" AndAlso File.Exists(soilDepthFPN) Then
            grmPrj.ReadSoilDepthFile(soilDepthFPN, True)
            msoilDepthFPN = soilDepthFPN
        End If
        If iniSoilSaturationFPN <> "" AndAlso File.Exists(iniSoilSaturationFPN) Then
            grmPrj.ReadLayerInitialSoilSaturation(iniSoilSaturationFPN, True)
            miniSoilSaturationFPN = iniSoilSaturationFPN
        End If
        If iniChannelFlowFPN <> "" AndAlso File.Exists(iniChannelFlowFPN) Then
            grmPrj.ReadLayerInitialChannelFlow(iniChannelFlowFPN, True)
            miniChannelFlowFPN = iniChannelFlowFPN
        End If
        grmPrj.SetGridNetworkFlowInformation()
        grmPrj.InitControlVolumeAttribute()
    End Sub

    Public Function IsInWatershedArea(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Boolean
        Dim id As Integer = grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).WSID
        If id > 0 Then
            Return True
        Else
            Return False
        End If
    End Function

    Public Function mostDownStreamCellArrayXColPosition() As Integer
        Return grmPrj.CV(grmPrj.mMostDownCellArrayNumber).XCol
    End Function

    Public Function mostDownStreamCellArrayYRowPosition() As Integer
        Return grmPrj.CV(grmPrj.mMostDownCellArrayNumber).YRow
    End Function

    ''' <summary>
    ''' Watershed ID list.
    ''' </summary>
    ''' <returns></returns>
    Public Function WSIDsAll() As List(Of Integer)
        Return grmPrj.WSNetwork.WSIDsAll
    End Function

    ''' <summary>
    ''' Watershed count.
    ''' </summary>
    ''' <returns></returns>
    Public Function WScount() As Integer
        Return grmPrj.WSNetwork.WSIDsAll.Count
    End Function


    Public Function upStreamWSIDs(currentWSID As Integer) As List(Of Integer)
        Return grmPrj.WSNetwork.WSIDsAllUps(currentWSID)
    End Function

    Public Function upStreamWSCount(currentWSID As Integer) As Integer
        Return grmPrj.WSNetwork.WSIDsAllUps(currentWSID).Count
    End Function

    Public Function downStreamWSIDs(currentWSID As Integer) As List(Of Integer)
        Return grmPrj.WSNetwork.WSIDsAllDowns(currentWSID)
    End Function

    Public Function downStreamWSCount(currentWSID As Integer) As Integer
        Return grmPrj.WSNetwork.WSIDsAllDowns(currentWSID).Count
    End Function

    Public Function watershedID(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Integer
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).WSID
        Else
            Return Nothing
        End If
    End Function

    Public Function flowDirection(colXArrayIdx As Integer, rowYArrayIdx As Integer) As String
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).FDir.ToString
        Else
            Return Nothing
        End If
    End Function

    Public Function flowAccumulation(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Integer
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).FAc
        Else
            Return Nothing
        End If
    End Function

    Public Function slope(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Double
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).Slope
        Else
            Return Nothing
        End If
    End Function

    Public Function streamValue(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Integer
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True AndAlso mstreamFPN <> "" Then
            If grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).IsStream Then
                Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).mStreamAttr.ChStrOrder
            End If
        End If
        Return Nothing
    End Function

    Public Function cellFlowType(colXArrayIdx As Integer, rowYArrayIdx As Integer) As String
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).FlowType.ToString
        Else
            Return Nothing
        End If
    End Function

    Public Function landCoverValue(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Integer
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).LandCoverValue
        Else
            Return Nothing
        End If
    End Function

    Public Function soilTextureVlaue(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Integer
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).SoilTextureValue
        Else
            Return Nothing
        End If
    End Function


    Public Function soilDepthVlaue(colXArrayIdx As Integer, rowYArrayIdx As Integer) As Integer
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Return grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).SoilDepthTypeValue
        Else
            Return Nothing
        End If
    End Function


    ''' <summary>
    '''  Select all cells in upstream area of a input cell position. Return string array of cell positions - "column, row".
    ''' </summary>
    ''' <param name="colXArrayIdx"></param>
    ''' <param name="rowYArrayIdx"></param>
    ''' <returns></returns>
    Public Function allCellsInUpstreamArea(colXArrayIdx As Integer, rowYArrayIdx As Integer) As String()
        If IsInWatershedArea(colXArrayIdx, rowYArrayIdx) = True Then
            Dim cvids As New List(Of Integer)
            Dim startingBaseCVID As Integer = grmPrj.WSCells(colXArrayIdx, rowYArrayIdx).CVID
            cvids = grmPrj.getAllUpstreamCells(startingBaseCVID)
            If cvids IsNot Nothing Then
                Dim cellsArray(cvids.Count - 1) As String
                Dim idx As Integer = 0
                For Each cvid As Integer In cvids
                    Dim colx As Integer = grmPrj.CVs(cvid - 1).XCol
                    Dim rowy As Integer = grmPrj.CVs(cvid - 1).YRow
                    Dim cellpos As String = colx.ToString + ", " + rowy.ToString
                    cellsArray(idx) = cellpos
                    idx += 1
                Next
                Return cellsArray
            Else
                Return Nothing
            End If
        Else
            Return Nothing
        End If
    End Function

    ''' <summary>
    '''  This method is applied to update all the subwatersheds parameters when there are more than 1 subwatershed.
    '''  Before this method is called, user set parameters must have been updated for each user set watershed.
    '''  And after this method is called, all the paramters in all the watersheds would be updated by using user set parameters.
    ''' </summary>
    Public Sub UpdateSubWatershedParametersUsingNetwork()
        If WScount() > 1 Then
            cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj)
        End If
    End Sub


    '여기에 필요한 내용 계속 추가
    ' function으로  return 한다.
    ' property로 하니까, 파이썬에서 앞에 get_이라는 문구가 붙어서 번거롭다.

End Class
