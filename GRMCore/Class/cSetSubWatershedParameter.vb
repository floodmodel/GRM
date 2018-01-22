Imports GRM

Public Class cSetSubWatershedParameter
    'Implements ioProjectFile

    '''' <summary>
    '''' 초기 포화도
    '''' </summary>
    '''' <remarks></remarks>
    'Public mIniSaturations As New Dictionary(Of Integer, Single)
    'Public mMinSlopeOFs As New Dictionary(Of Integer, Single)
    'Public mMinSlopeChBeds As New Dictionary(Of Integer, Single)
    'Public mMinChBaseWidths As New Dictionary(Of Integer, Single)
    'Public mChRoughnesses As New Dictionary(Of Integer, Single)
    'Public mDryStreamOrders As New Dictionary(Of Integer, Integer)
    'Public mIniFlows As New Dictionary(Of Integer, Nullable(Of Single))

    'Public mCalCoefLCRoughnesses As New Dictionary(Of Integer, Single)
    'Public mCalCoefPorosities As New Dictionary(Of Integer, Single)
    'Public mCalCoefWFSuctionHeads As New Dictionary(Of Integer, Single)
    'Public mCalCoefHydraulicKs As New Dictionary(Of Integer, Single)
    'Public mCalCoefSoilDepths As New Dictionary(Of Integer, Single)

    'Public mUserSet As New Dictionary(Of Integer, Boolean)

    Public userPars As New Dictionary(Of Integer, cUserParameters)

    Public Sub New()
        'mIniSaturations.Clear()
        'mMinSlopeOFs.Clear()
        'mMinSlopeChBeds.Clear()
        'mMinSlopeChBeds.Clear()
        'mChRoughnesses.Clear()
        'mDryStreamOrders.Clear()
        'mIniFlows.Clear()
        'mCalCoefLCRoughnesses.Clear()
        'mCalCoefPorosities.Clear()
        'mCalCoefWFSuctionHeads.Clear()
        'mCalCoefHydraulicKs.Clear()
        'mCalCoefSoilDepths.Clear()
        'mUserSet.Clear()
        userPars.Clear()
    End Sub

    Public Sub SetSubWSkeys(ByVal WSList As List(Of Integer))
        For Each wsid As Integer In WSList
            If userPars.ContainsKey(wsid) = False Then userPars.Add(wsid, Nothing)
            'If mIniSaturations.ContainsKey(wsid) = False Then mIniSaturations.Add(wsid, Nothing)
            'If mMinSlopeOFs.ContainsKey(wsid) = False Then mMinSlopeOFs.Add(wsid, Nothing)
            'If mMinSlopeChBeds.ContainsKey(wsid) = False Then mMinSlopeChBeds.Add(wsid, Nothing)
            'If mMinChBaseWidths.ContainsKey(wsid) = False Then mMinChBaseWidths.Add(wsid, Nothing)
            'If mChRoughnesses.ContainsKey(wsid) = False Then mChRoughnesses.Add(wsid, Nothing)
            'If mDryStreamOrders.ContainsKey(wsid) = False Then mDryStreamOrders.Add(wsid, Nothing)
            'If mIniFlows.ContainsKey(wsid) = False Then mIniFlows.Add(wsid, Nothing)
            'If mCalCoefLCRoughnesses.ContainsKey(wsid) = False Then mCalCoefLCRoughnesses.Add(wsid, Nothing)
            'If mCalCoefPorosities.ContainsKey(wsid) = False Then mCalCoefPorosities.Add(wsid, Nothing)
            'If mCalCoefWFSuctionHeads.ContainsKey(wsid) = False Then mCalCoefWFSuctionHeads.Add(wsid, Nothing)
            'If mCalCoefHydraulicKs.ContainsKey(wsid) = False Then mCalCoefHydraulicKs.Add(wsid, Nothing)
            'If mCalCoefSoilDepths.ContainsKey(wsid) = False Then mCalCoefSoilDepths.Add(wsid, Nothing)
            'If mUserSet.ContainsKey(wsid) = False Then mUserSet.Add(wsid, False)
        Next
    End Sub

    Public ReadOnly Property IsUserSetSubWSexist() As Boolean
        Get
            For Each k As Integer In userPars.Keys ' mIniSaturations.Keys
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

    Public ReadOnly Property IsSet As Boolean 'Implements ioProjectFile.IsSet
        Get
            Return Not (userPars Is Nothing)
        End Get
    End Property

    Public Sub RemoveWSParametersSet(ByVal WSID As Integer)
        userPars(WSID) = Nothing
        'mIniSaturations(WSID) = Nothing
        'mMinSlopeOFs(WSID) = Nothing
        'mMinSlopeChBeds(WSID) = Nothing
        'mMinChBaseWidths(WSID) = Nothing
        'mChRoughnesses(WSID) = Nothing
        'mDryStreamOrders(WSID) = Nothing
        'mIniFlows(WSID) = Nothing
        'mCalCoefLCRoughnesses(WSID) = Nothing
        'mCalCoefPorosities(WSID) = Nothing
        'mCalCoefWFSuctionHeads(WSID) = Nothing
        'mCalCoefHydraulicKs(WSID) = Nothing
        'mCalCoefSoilDepths(WSID) = Nothing
        'mUserSet(WSID) = False
    End Sub



    Public Sub GetValues(prjDB As GRMProject) 'Implements ioProjectFile.GetValues
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
                        upars.isUserSet = .UserSet
                    End If
                End With
                userPars.Add(row.ID, upars)
            Next
        End If
    End Sub

    Public Sub SetValues(prjDB As GRMProject) 'Implements ioProjectFile.SetValues
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
                    .UserSet = userPars(wsid).isUserSet
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
            'If Not grmproject.mSimulationType = cGRM.SimulationType.RealTime Then
            .minSlopeOF = spars(rid).minSlopeOF
            .minChBaseWidth = spars(rid).minChBaseWidth
            .chRoughness = spars(rid).chRoughness
            .dryStreamOrder = spars(rid).dryStreamOrder
            .ccLCRoughness = spars(rid).ccLCRoughness
            .ccPorosity = spars(rid).ccPorosity
            .ccWFSuctionHead = spars(rid).ccWFSuctionHead
            .ccHydraulicK = spars(rid).ccHydraulicK
            .ccSoilDepth = spars(rid).ccSoilDepth
            'End If
        End With
        Return True
    End Function

End Class
