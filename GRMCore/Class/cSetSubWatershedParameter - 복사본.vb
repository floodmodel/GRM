Imports GRM

Public Class cSetSubWatershedParameter
    Implements ioProjectFile

    ''' <summary>
    ''' 초기 포화도
    ''' </summary>
    ''' <remarks></remarks>
    Public mIniSaturations As New Dictionary(Of Integer, Single)
    Public mMinSlopeOFs As New Dictionary(Of Integer, Single)
    Public mMinSlopeChBeds As New Dictionary(Of Integer, Single)
    Public mMinChBaseWidths As New Dictionary(Of Integer, Single)
    Public mChRoughnesses As New Dictionary(Of Integer, Single)
    Public mDryStreamOrders As New Dictionary(Of Integer, Integer)
    Public mIniFlows As New Dictionary(Of Integer, Nullable(Of Single))

    Public mCalCoefLCRoughnesses As New Dictionary(Of Integer, Single)
    Public mCalCoefPorosities As New Dictionary(Of Integer, Single)
    Public mCalCoefWFSuctionHeads As New Dictionary(Of Integer, Single)
    Public mCalCoefHydraulicKs As New Dictionary(Of Integer, Single)
    Public mCalCoefSoilDepths As New Dictionary(Of Integer, Single)

    Public mUserSet As New Dictionary(Of Integer, Boolean)

    Public Sub New()
        mIniSaturations.Clear()
        mMinSlopeOFs.Clear()
        mMinSlopeChBeds.Clear()
        mMinSlopeChBeds.Clear()
        mChRoughnesses.Clear()
        mDryStreamOrders.Clear()
        mIniFlows.Clear()
        mCalCoefLCRoughnesses.Clear()
        mCalCoefPorosities.Clear()
        mCalCoefWFSuctionHeads.Clear()
        mCalCoefHydraulicKs.Clear()
        mCalCoefSoilDepths.Clear()
        mUserSet.Clear()
    End Sub

    Public Sub SetSubWSkeys(ByVal WSList As List(Of Integer))
        For Each wsid As Integer In WSList
            If mIniSaturations.ContainsKey(wsid) = False Then mIniSaturations.Add(wsid, Nothing)
            If mMinSlopeOFs.ContainsKey(wsid) = False Then mMinSlopeOFs.Add(wsid, Nothing)
            If mMinSlopeChBeds.ContainsKey(wsid) = False Then mMinSlopeChBeds.Add(wsid, Nothing)
            If mMinChBaseWidths.ContainsKey(wsid) = False Then mMinChBaseWidths.Add(wsid, Nothing)
            If mChRoughnesses.ContainsKey(wsid) = False Then mChRoughnesses.Add(wsid, Nothing)
            If mDryStreamOrders.ContainsKey(wsid) = False Then mDryStreamOrders.Add(wsid, Nothing)
            If mIniFlows.ContainsKey(wsid) = False Then mIniFlows.Add(wsid, Nothing)
            If mCalCoefLCRoughnesses.ContainsKey(wsid) = False Then mCalCoefLCRoughnesses.Add(wsid, Nothing)
            If mCalCoefPorosities.ContainsKey(wsid) = False Then mCalCoefPorosities.Add(wsid, Nothing)
            If mCalCoefWFSuctionHeads.ContainsKey(wsid) = False Then mCalCoefWFSuctionHeads.Add(wsid, Nothing)
            If mCalCoefHydraulicKs.ContainsKey(wsid) = False Then mCalCoefHydraulicKs.Add(wsid, Nothing)
            If mCalCoefSoilDepths.ContainsKey(wsid) = False Then mCalCoefSoilDepths.Add(wsid, Nothing)
            If mUserSet.ContainsKey(wsid) = False Then mUserSet.Add(wsid, False)
        Next
    End Sub

    Public ReadOnly Property IsUserSetSubWSexist() As Boolean
        Get
            For Each k As Integer In mIniSaturations.Keys
                If mUserSet(k) = True Then
                    Return True
                End If
            Next
            Return False
        End Get
    End Property

    Public ReadOnly Property AllSubWSParametersAreSet() As Boolean
        Get
            For Each k As Integer In mIniSaturations.Keys
                If mIniSaturations(k) = Nothing Then
                    Return False
                End If
            Next
            Return True
        End Get
    End Property

    Public ReadOnly Property NowSubWSParameterIsSet(ByVal WSID As Integer) As Boolean
        Get
            Return Not (mIniSaturations(WSID) = Nothing)
        End Get
    End Property

    Public ReadOnly Property IsSet As Boolean Implements ioProjectFile.IsSet
        Get
            Throw New NotImplementedException()
        End Get
    End Property

    Public Sub RemoveWSParametersSet(ByVal WSID As Integer)
        mIniSaturations(WSID) = Nothing
        mMinSlopeOFs(WSID) = Nothing
        mMinSlopeChBeds(WSID) = Nothing
        mMinChBaseWidths(WSID) = Nothing
        mChRoughnesses(WSID) = Nothing
        mDryStreamOrders(WSID) = Nothing
        mIniFlows(WSID) = Nothing
        mCalCoefLCRoughnesses(WSID) = Nothing
        mCalCoefPorosities(WSID) = Nothing
        mCalCoefWFSuctionHeads(WSID) = Nothing
        mCalCoefHydraulicKs(WSID) = Nothing
        mCalCoefSoilDepths(WSID) = Nothing
        mUserSet(WSID) = False
    End Sub



    Public Sub GetValues(prjDB As GRMProject) Implements ioProjectFile.GetValues
        Dim dt As GRMProject.SubWatershedSettingsDataTable = prjDB.SubWatershedSettings
        If dt.Rows.Count > 0 Then
            For Each row As GRMProject.SubWatershedSettingsRow In dt
                With row
                    If Not .IsIDNull Then
                        mIniSaturations.Add(row.ID, .IniSaturation)
                        mMinSlopeOFs.Add(row.ID, .MinSlopeOF)
                        mMinSlopeChBeds.Add(row.ID, .MinSlopeChBed)
                        mMinChBaseWidths.Add(row.ID, .MinChBaseWidth)
                        mChRoughnesses.Add(row.ID, .ChRoughness)
                        mDryStreamOrders.Add(row.ID, .DryStreamOrder)
                        If .IsIniFlowNull Then
                            mIniFlows.Add(row.ID, Nothing)
                        Else
                            mIniFlows.Add(row.ID, .IniFlow)
                        End If
                        mCalCoefLCRoughnesses.Add(row.ID, .CalCoefLCRoughness)
                        mCalCoefPorosities.Add(row.ID, .CalCoefPorosity)
                        mCalCoefWFSuctionHeads.Add(row.ID, .CalCoefWFSuctionHead)
                        mCalCoefHydraulicKs.Add(row.ID, .CalCoefHydraulicK)
                        mCalCoefSoilDepths.Add(row.ID, .CalCoefSoilDepth)
                        mUserSet.Add(row.ID, .UserSet)
                    End If
                End With
            Next
        End If
    End Sub

    Public Sub SetValues(prjDB As GRMProject) Implements ioProjectFile.SetValues
        Dim dt As GRMProject.SubWatershedSettingsDataTable = prjDB.SubWatershedSettings
        If Not (AllSubWSParametersAreSet = False) Then
            dt.Rows.Clear()
            For Each wsid As Integer In mIniSaturations.Keys
                Dim newRow As GRMProject.SubWatershedSettingsRow = dt.NewSubWatershedSettingsRow
                With newRow
                    .ID = wsid
                    .IniSaturation = mIniSaturations(wsid)
                    .MinSlopeOF = mMinSlopeOFs(wsid)
                    .ChRoughness = mChRoughnesses(wsid)
                    .MinSlopeChBed = mMinSlopeChBeds(wsid)
                    .MinChBaseWidth = mMinChBaseWidths(wsid)
                    .DryStreamOrder = mDryStreamOrders(wsid)
                    If mIniFlows(wsid) Is Nothing Then
                        .SetIniFlowNull()
                    Else
                        .IniFlow = mIniFlows(wsid).Value
                    End If
                    .CalCoefLCRoughness = mCalCoefLCRoughnesses(wsid)
                    .CalCoefPorosity = mCalCoefPorosities(wsid)
                    .CalCoefWFSuctionHead = mCalCoefWFSuctionHeads(wsid)
                    .CalCoefHydraulicK = mCalCoefHydraulicKs(wsid)
                    .CalCoefSoilDepth = mCalCoefSoilDepths(wsid)
                    .UserSet = mUserSet(wsid)
                End With
                dt.Rows.Add(newRow)
            Next
        End If
    End Sub


    Public Shared Sub UpdateSubWSParametersForWSNetwork(ByVal grmProject As cProject)
        With grmProject
            For Each wsid As Integer In .Watershed.WSIDList
                If .SubWSPar.mUserSet(wsid) = True Then
                    Dim wsidToExclude As New List(Of Integer)
                    For Each upsid As Integer In .WSNetwork.WSIDsAllUps(wsid)
                        If .SubWSPar.mUserSet(upsid) = True Then
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
        With grmproject.SubWSPar
            .mIniSaturations(TargetWSid) = .mIniSaturations(ReferenceWSid)
            .mMinSlopeChBeds(TargetWSid) = .mMinSlopeChBeds(ReferenceWSid)

            If Not (grmproject.mSimulationType = cGRM.SimulationType.RealTimeLauncher _
                OrElse grmproject.mSimulationType = cGRM.SimulationType.RealTime) Then
                .mMinSlopeOFs(TargetWSid) = .mMinSlopeOFs(ReferenceWSid)

                .mMinChBaseWidths(TargetWSid) = .mMinChBaseWidths(ReferenceWSid)
                .mChRoughnesses(TargetWSid) = .mChRoughnesses(ReferenceWSid)
                .mDryStreamOrders(TargetWSid) = .mDryStreamOrders(ReferenceWSid)

                .mCalCoefLCRoughnesses(TargetWSid) = .mCalCoefLCRoughnesses(ReferenceWSid)
                .mCalCoefPorosities(TargetWSid) = .mCalCoefPorosities(ReferenceWSid)
                .mCalCoefWFSuctionHeads(TargetWSid) = .mCalCoefWFSuctionHeads(ReferenceWSid)
                .mCalCoefHydraulicKs(TargetWSid) = .mCalCoefHydraulicKs(ReferenceWSid)
                .mCalCoefSoilDepths(TargetWSid) = .mCalCoefSoilDepths(ReferenceWSid)
            End If
        End With
        Return True
    End Function

End Class
