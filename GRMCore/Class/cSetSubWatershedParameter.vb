
Public Class cSetSubWatershedParameter
    Public userPars As New Dictionary(Of Integer, cUserParameters)

    Public Sub New()
        userPars.Clear()
    End Sub

    Public Sub SetSubWSkeys(ByVal WSList As List(Of Integer))
        For Each wsid As Integer In WSList
            If userPars.ContainsKey(wsid) = False Then
                Dim upars As New cUserParameters
                userPars.Add(wsid, upars)
            End If
        Next
    End Sub

    Public ReadOnly Property IsUserSetSubWSexist() As Boolean
        Get
            For Each k As Integer In userPars.Keys
                If userPars(k).isUserSet = True Then
                    Return True
                End If
            Next
            Return False
        End Get
    End Property

    Public ReadOnly Property AllSubWSParametersAreSet() As Boolean
        Get
            For Each k As Integer In userPars.Keys
                If userPars(k).iniSaturation = Nothing Then
                    Return False
                End If
            Next
            Return True
        End Get
    End Property

    Public ReadOnly Property NowSubWSParameterIsSet(ByVal WSID As Integer) As Boolean
        Get
            Return Not (userPars(WSID).iniSaturation = Nothing)
        End Get
    End Property

    Public ReadOnly Property IsSet As Boolean
        Get
            Return Not (userPars Is Nothing)
        End Get
    End Property

    Public Sub RemoveWSParametersSet(ByVal WSID As Integer)
        userPars(WSID) = Nothing
    End Sub


    Public Sub GetValues(prjDB As GRMProject)
        Dim dt As GRMProject.SubWatershedSettingsDataTable = prjDB.SubWatershedSettings
        If dt.Rows.Count > 0 Then
            For Each row As GRMProject.SubWatershedSettingsRow In dt
                Dim upars As New cUserParameters
                With row
                    If Not .IsIDNull Then
                        upars.wsid = row.ID
                        upars.iniSaturation = .IniSaturation
                        upars.minSlopeOF = .MinSlopeOF
                        upars.minSlopeChBed = .MinSlopeChBed
                        upars.minChBaseWidth = .MinChBaseWidth
                        upars.chRoughness = .ChRoughness
                        upars.dryStreamOrder = .DryStreamOrder
                        If .IsIniFlowNull Then
                            upars.iniFlow = Nothing
                        Else
                            upars.iniFlow = .IniFlow
                        End If
                        upars.ccLCRoughness = .CalCoefLCRoughness
                        upars.ccPorosity = .CalCoefPorosity
                        upars.ccWFSuctionHead = .CalCoefWFSuctionHead
                        upars.ccHydraulicK = .CalCoefHydraulicK
                        upars.ccSoilDepth = .CalCoefSoilDepth
                        If .IsPowerCeofUnSaturatedKNull Then
                            upars.expUnsaturatedK = cGRM.CONST_EXPONENTIAL_NUMBER_UNSATURATED_K
                        Else
                            upars.expUnsaturatedK = .PowerCeofUnSaturatedK
                        End If
                        If LCase(.UserSet) = "true" Then
                            upars.isUserSet = True
                        Else
                            upars.isUserSet = False
                        End If

                    End If
                End With
                userPars.Add(row.ID, upars)
            Next
        End If
    End Sub

    Public Sub SetValues(prjDB As GRMProject)
        Dim dt As GRMProject.SubWatershedSettingsDataTable = prjDB.SubWatershedSettings
        If Not (AllSubWSParametersAreSet = False) Then
            dt.Rows.Clear()
            For Each wsid As Integer In userPars.Keys
                Dim newRow As GRMProject.SubWatershedSettingsRow = dt.NewSubWatershedSettingsRow
                With newRow
                    .ID = wsid
                    .IniSaturation = userPars(wsid).iniSaturation
                    .MinSlopeOF = userPars(wsid).minSlopeOF
                    .ChRoughness = userPars(wsid).chRoughness
                    .MinSlopeChBed = userPars(wsid).minSlopeChBed
                    .MinChBaseWidth = userPars(wsid).minChBaseWidth
                    .DryStreamOrder = userPars(wsid).dryStreamOrder
                    If userPars(wsid).iniFlow Is Nothing Then
                        .SetIniFlowNull()
                    Else
                        .IniFlow = userPars(wsid).iniFlow.Value
                    End If
                    .CalCoefLCRoughness = userPars(wsid).ccLCRoughness
                    .CalCoefPorosity = userPars(wsid).ccPorosity
                    .CalCoefWFSuctionHead = userPars(wsid).ccWFSuctionHead
                    .CalCoefHydraulicK = userPars(wsid).ccHydraulicK
                    .CalCoefSoilDepth = userPars(wsid).ccSoilDepth
                    .PowerCeofUnSaturatedK = userPars(wsid).expUnsaturatedK
                    .UserSet = userPars(wsid).isUserSet.ToString
                End With
                dt.Rows.Add(newRow)
            Next
        End If
    End Sub


    Public Shared Sub UpdateSubWSParametersForWSNetwork(ByVal grmProject As cProject)
        With grmProject
            For Each wsid As Integer In .Watershed.WSIDList
                If .SubWSPar.userPars(wsid).isUserSet = True Then
                    Dim wsidToExclude As New List(Of Integer)
                    For Each upsid As Integer In .WSNetwork.WSIDsAllUps(wsid)
                        If .SubWSPar.userPars(upsid).isUserSet = True Then
                            If Not wsidToExclude.Contains(upsid) Then
                                wsidToExclude.Add(upsid)
                            End If
                            For Each upupID As Integer In .WSNetwork.WSIDsAllUps(upsid)
                                If Not wsidToExclude.Contains(upupID) Then
                                    wsidToExclude.Add(upupID)
                                End If
                            Next
                        End If
                    Next

                    For Each upsid As Integer In .WSNetwork.WSIDsAllUps(wsid)
                        If wsidToExclude.Contains(upsid) = False Then
                            SetWSParametersWithAnotherWatershedParameterSet(grmProject, upsid, wsid)
                        End If
                    Next
                End If
            Next
        End With
    End Sub


    Private Shared Function SetWSParametersWithAnotherWatershedParameterSet(ByVal grmproject As cProject, ByVal TargetWSid As Integer, ByVal ReferenceWSid As Integer) As Boolean
        Dim spars As New Dictionary(Of Integer, cUserParameters)
        spars = grmproject.SubWSPar.userPars
        Dim rid As Integer = ReferenceWSid
        With grmproject.SubWSPar.userPars(TargetWSid)
            .iniSaturation = spars(rid).iniSaturation
            .minSlopeChBed = spars(rid).minSlopeChBed
            .minSlopeOF = spars(rid).minSlopeOF
            .minChBaseWidth = spars(rid).minChBaseWidth
            .chRoughness = spars(rid).chRoughness
            .dryStreamOrder = spars(rid).dryStreamOrder
            .ccLCRoughness = spars(rid).ccLCRoughness
            .ccPorosity = spars(rid).ccPorosity
            .ccWFSuctionHead = spars(rid).ccWFSuctionHead
            .ccHydraulicK = spars(rid).ccHydraulicK
            .ccSoilDepth = spars(rid).ccSoilDepth
        End With
        Return True
    End Function

End Class
