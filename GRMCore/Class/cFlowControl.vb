Imports System.Runtime.Serialization.Formatters.Binary

Public Class cFlowControl
    Implements ICloneable
    Public Enum FCDataSourceType
        UserEdit
        Constant
        TextFile
        ReservoirOperation
    End Enum

    Public Enum FlowControlType
        ReservoirOutflow '상류모의, 저류량 고려하지 않고, 댐에서의 방류량만 고려함
        Inlet  '상류 모의하지 않는것. 저류량 고려하지 않고, inlet grid에서의 outfow 만 고려함.
        SinkFlow '상류모의, 입력된 sink flow data 고려함. 저수지 고려안함.
        SourceFlow '상류모의, 입력된 source flow data 고려함. 저수지 고려안함.
        ReservoirOperation '상류모의, 저수지 고려, 방류량은 operation rule에 의해서 결정됨. 사용자 입력 인터페이스 구현하지 않음.
        ' 저류량-방류량, 유입량-방류량 관계식을 이용해서 소스코드에 반영 가능
        NONE
    End Enum

    Public Enum ROType
        AutoROM
        RigidROM
        ConstantQ
        SDEqation
    End Enum

    Public Structure FCData
        Dim CVID As Integer
        Dim Order As Integer
        Dim Value As Single
        Dim DataTime As String
    End Structure

    Public Shared ReadOnly DEFAULT_FLOW_CONTROL_COLOR As Color = Color.Maroon
    Private mFVMsolver As New cFVMSolver

    Public mdtFCGridInfo As GRMProject.FlowControlGridDataTable
    Public mdtFCFlowData As DataTable
    Public mCellColor As Color

    ''' <summary>
    ''' 현재의 모델링 시간(t)에 적용된 flow control data 값
    ''' </summary>
    ''' <remarks></remarks>
    Public mFCdataToApplyNowT As SortedList(Of Integer, Double)
    Private mInletExisted As Boolean = False
    Private mInletList As List(Of Integer)

    ''' <summary>
    ''' 현재 계산 시간에서 적용된 fc 자료
    ''' </summary>
    ''' <remarks></remarks>
    Private mFCGridCVidList As List(Of Integer)


    Public ReadOnly Property FCCellCount() As Integer
        Get
            Return mdtFCGridInfo.Rows.Count
        End Get
    End Property


    Public Sub GetValues(ByVal prj As cProject)
        Dim cc As New ColorConverter
        Dim row As GRMProject.ProjectSettingsRow = CType(prj.PrjFile.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        mdtFCGridInfo = New GRMProject.FlowControlGridDataTable
        mdtFCGridInfo = prj.PrjFile.FlowControlGrid
        If prj.mSimulationType = cGRM.SimulationType.SingleEvent Then
            mdtFCFlowData = New DataTable '원 :2017.6.20 부터 사용
            mdtFCFlowData.Columns.Add(New Global.System.Data.DataColumn("CVID", GetType(Integer), Nothing, Global.System.Data.MappingType.Element))
            mdtFCFlowData.Columns.Add(New Global.System.Data.DataColumn("Value", GetType(Single), Nothing, Global.System.Data.MappingType.Element))
            mdtFCFlowData.Columns.Add(New Global.System.Data.DataColumn("DataTime", GetType(String), Nothing, Global.System.Data.MappingType.Element))
        End If
    End Sub


    Public ReadOnly Property IsSet() As Boolean
        Get
            If mdtFCGridInfo Is Nothing Then Return False
            If mdtFCGridInfo.Rows.Count > 0 Then Return True
            Return False
        End Get
    End Property

    ''' <summary>
    ''' Flow control 설정 대상 격자의 CVID 정보를 업데이트 한다.
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub UpdateFCGridInfoAndData(prj As cProject)
        mInletList = New List(Of Integer)
        mFCGridCVidList = New List(Of Integer)
        mFCdataToApplyNowT = New SortedList(Of Integer, Double)
        For Each r As GRMProject.FlowControlGridRow In mdtFCGridInfo
            r.CVID = prj.WSCells(r.ColX, r.RowY).CVID
            mFCGridCVidList.Add(r.CVID)
            If r.ControlType.ToString = FlowControlType.Inlet.ToString Then
                mInletList.Add(r.CVID)
            End If
            mFCdataToApplyNowT.Add(r.CVID, Nothing)
            If prj.mSimulationType = cGRM.SimulationType.SingleEvent Then
                If r.ControlType <> cFlowControl.FlowControlType.ReservoirOperation.ToString Then
                    If File.Exists(r.FlowDataFile) Then
                        Dim Lines() As String = System.IO.File.ReadAllLines(r.FlowDataFile)
                        For n As Integer = 0 To Lines.Length - 1
                            Dim ar As DataRow = prj.FCGrid.mdtFCFlowData.NewRow
                            ar("CVID") = r.CVID
                            ar("Value") = CSng(Lines(n))
                            If prj.GeneralSimulEnv.mIsDateTimeFormat = True Then
                                ar("DataTime") = cComTools.GetTimeToPrintOut(True, prj.GeneralSimulEnv.mSimStartDateTime, CInt(r.DT) * n)
                            Else
                                ar("DataTime") = CStr(CInt(r.DT) * n)
                            End If
                            mdtFCFlowData.Rows.Add(ar)
                        Next
                    End If
                End If
            End If
        Next
        If mInletList.Count > 0 Then mInletExisted = True
    End Sub


    Public ReadOnly Property InletExisted() As Boolean
        Get
            Return mInletExisted
        End Get
    End Property

    ''' <summary>
    ''' Flow control 대상 격자 중 inlet 격자의 CVID 목록
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property InletCVidList() As List(Of Integer)
        Get
            Return mInletList
        End Get
    End Property

    ''' <summary>
    ''' Flow control 대상 격자의 CVID 목록
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property FCGridCVidList() As List(Of Integer)
        Get
            Return mFCGridCVidList
        End Get
    End Property

    Public ReadOnly Property GetFCName(cvid As Integer) As String
        Get
            For Each r As GRMProject.FlowControlGridRow In mdtFCGridInfo.Rows
                If r.CVID = cvid Then Return r.Name
            Next
            Return ""
        End Get
    End Property

    ''' <summary>
    ''' 계산하면서 업데이트 되는 자료만 백업한다.
    ''' </summary>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function Clone() As Object Implements ICloneable.Clone
        Dim cln As New cFlowControl
        cln.mdtFCGridInfo = New GRMProject.FlowControlGridDataTable
        If mdtFCGridInfo IsNot Nothing Then
            cln.mdtFCFlowData = New DataTable
            cln.mFCdataToApplyNowT = New SortedList(Of Integer, Double)
            cln.mdtFCGridInfo = CType(Me.mdtFCGridInfo.Clone, GRMProject.FlowControlGridDataTable)
            If mdtFCFlowData IsNot Nothing Then
                cln.mdtFCFlowData = CType(Me.mdtFCFlowData.Clone, DataTable)
            End If
            If Me.mFCdataToApplyNowT IsNot Nothing Then
                For Each k As Integer In Me.mFCdataToApplyNowT.Keys
                    cln.mFCdataToApplyNowT.Add(k, Me.mFCdataToApplyNowT(k))
                Next
            End If
        End If
        Return cln
    End Function


    Sub CalFCReservoirOperation(ByVal project As cProject, ByVal cvan As Integer, ByVal nowT_MIN As Integer)
        Dim dtsec As Integer = cThisSimulation.dtsec
        Dim fcCVid As Integer = project.CV(cvan).CVID
        Dim Rows() As DataRow = project.FCGrid.mdtFCGridInfo.Select(String.Format("cvid = {0}", fcCVid))
        Dim fcRow As GRMProject.FlowControlGridRow = CType(Rows(0), GRMProject.FlowControlGridRow)
        Dim rowOrder As Integer
        Dim QforSinkOrSourceFlow As Single = 0
        Dim cellsize As Integer = project.Watershed.mCellSize
        With project.CV(cvan)
            If .FCType = cFlowControl.FlowControlType.SinkFlow OrElse .FCType = cFlowControl.FlowControlType.SourceFlow Then
                rowOrder = CInt((nowT_MIN - 1) \ CInt(fcRow.DT))
                Dim dv As DataView
                dv = New DataView(project.FCGrid.mdtFCFlowData, String.Format("cvid={0}", fcCVid), " datetime asc", DataViewRowState.CurrentRows)
                Dim dt As DataTable = dv.Table
                Dim fcdt As DataTable = CType(dt, DataTable)
                QforSinkOrSourceFlow = CSng(fcdt.Rows(rowOrder).Item("Value"))
            End If
            Select Case .FCType
                Case cFlowControl.FlowControlType.SinkFlow
                    .StorageCumulative_m3 = .StorageCumulative_m3 + .StorageAddedForDTfromRF_m3 + .QsumCVw_dt_m3 - QforSinkOrSourceFlow
                Case cFlowControl.FlowControlType.SourceFlow
                    .StorageCumulative_m3 = .StorageCumulative_m3 + .StorageAddedForDTfromRF_m3 + .QsumCVw_dt_m3 + QforSinkOrSourceFlow
                Case Else
                    .StorageCumulative_m3 = .StorageCumulative_m3 + .StorageAddedForDTfromRF_m3 + .QsumCVw_dt_m3
            End Select

            Select Case fcRow.ROType.ToString
                Case cFlowControl.ROType.AutoROM.ToString
                    Call ApplyReservoirAutoROM(project.CV(cvan), cellsize, CSng(fcRow.MaxStorage) * CSng(fcRow.MaxStorageR))
                Case cFlowControl.ROType.RigidROM.ToString
                    .StorageCumulative_m3 = CSng(.StorageCumulative_m3) - CSng(fcRow.ROConstQ) * dtsec
                    Call ApplyReservoirRigidROM(project.CV(cvan), cellsize, CSng(fcRow.MaxStorage) * CSng(fcRow.MaxStorageR), CSng(fcRow.ROConstQ))

                Case cFlowControl.ROType.ConstantQ.ToString
                    Dim bOurflowDuration As Boolean = False
                    If nowT_MIN <= CSng(fcRow.ROConstQDuration) * 60 Then
                        .StorageCumulative_m3 = CSng(.StorageCumulative_m3) - CSng(fcRow.ROConstQ) * dtsec
                        bOurflowDuration = True
                    End If
                    Call ApplyReservoirConstantDischarge(project.CV(cvan), cellsize, CSng(fcRow.ROConstQ),
                                                          CSng(fcRow.MaxStorage) * CSng(fcRow.MaxStorageR), bOurflowDuration)
                Case cFlowControl.ROType.SDEqation.ToString
            End Select
        End With

    End Sub

    Sub ApplyReservoirAutoROM(ByVal cv As cCVAttribute, ByVal cellsize As Integer, ByVal MaxStorageApp As Single)
        Dim sngQout_cms As Single
        Dim sngDY_m As Single = cellsize
        With cv
            Dim sngQinput_m3 As Single = .StorageAddedForDTfromRF_m3 + .QsumCVw_dt_m3
            If .StorageCumulative_m3 >= MaxStorageApp Then
                sngQout_cms = sngQinput_m3 / cThisSimulation.dtsec  '이때는 셀의 특성(크기, 경사, 조도 등...)에 상관없이 상류에서 유입되는 모든 양이 유출됨.
                .StorageCumulative_m3 = MaxStorageApp '누가저류량이 최대저류량 보다 같거나 크면, 더이상 누가되지 않고, 최대저류량을 유지한다.
            Else
                sngQout_cms = 0
            End If
        End With
        Call CalReservoirOutFlowInReservoirOperation(cv, sngQout_cms, sngDY_m)
    End Sub


    Sub ApplyReservoirRigidROM(ByVal cv As cCVAttribute, ByVal cellsize As Integer,
                               ByVal maxStorageApp As Single, ByVal RoQ_CONST_CMS As Single)
        Dim dy_m As Single = cellsize
        Dim sngQout_cms As Single
        With cv
            Dim sngQinput_m3 As Single = .StorageAddedForDTfromRF_m3 + .QsumCVw_dt_m3
            If .StorageCumulative_m3 >= maxStorageApp Then
                sngQout_cms = sngQinput_m3 / cThisSimulation.dtsec
                .StorageCumulative_m3 = maxStorageApp '누가저류량이 최대저류량 보다 같거나 크면, 더이상 누가되지 않고, 최대저류량을 유지한다.
            Else
                If .StorageCumulative_m3 < RoQ_CONST_CMS * cThisSimulation.dtsec Then
                    '이경우는 주어진 유출량으로 유출하지 못하고, 그것보다 작은 양으로 유출된다는 의미
                    '즉 dt 시간에서 저류된 모든 양이 유출되는 유량으로 현재 저수지에서의 유출량을 계산해야 한다.
                    '이건 이번 저류량을 계산하기 전의 저류량
                    Dim sngStorage_tM1 As Single = .StorageCumulative_m3
                    If sngStorage_tM1 < 0 Then
                        sngQout_cms = 0
                    Else
                        sngQout_cms = sngStorage_tM1 / cThisSimulation.dtsec
                    End If
                    .StorageCumulative_m3 = 0
                Else '이경우는 주어진 constant 유출량으로 유출 가능하다는 의미
                    sngQout_cms = RoQ_CONST_CMS
                End If
            End If
        End With
        Call CalReservoirOutFlowInReservoirOperation(cv, sngQout_cms, dy_m)
    End Sub


    Sub ApplyReservoirConstantDischarge(ByVal cv As cCVAttribute, cellsize As Integer, ByVal RoQ_CONST_CMS As Single,
                                        ByVal maxStorageApp As Single, ByVal bOutflowDuration As Boolean)
        Dim dy_m As Single = cellsize
        Dim sngQout_cms As Single
        With cv
            If bOutflowDuration = True Then
                If .StorageCumulative_m3 <= 0 Then
                    '이경우는 주어진 유출량으로 유출하지 못하고, 그것보다 작은 양으로 유출된다는 의미
                    '즉 dt 시간에서 저류된 모든 양이 유출되는 유량으로 현재 저수지에서의 유출량을 계산해야 한다.
                    Dim sngStorage_tM1 As Single = (RoQ_CONST_CMS * cThisSimulation.dtsec) + .StorageCumulative_m3
                    If sngStorage_tM1 < 0 Then
                        sngQout_cms = 0
                    Else
                        sngQout_cms = sngStorage_tM1 / cThisSimulation.dtsec
                    End If
                    .StorageCumulative_m3 = 0
                Else '이경우는 주어진 constant 유출량으로 유출 가능하다는 의미
                    sngQout_cms = RoQ_CONST_CMS
                    .StorageCumulative_m3 = .StorageCumulative_m3 - RoQ_CONST_CMS
                    If .StorageCumulative_m3 >= maxStorageApp Then
                        'Constant dischrage에서는 계속 누가 시킨다. 
                        '누가저류량이 최대저류량 보다 같거나 크면, 더이상 누가되지 않고, 최대저류량을 유지한다.
                        .StorageCumulative_m3 = maxStorageApp
                    End If
                End If
            Else
                sngQout_cms = 0
            End If
        End With
        Call CalReservoirOutFlowInReservoirOperation(cv, sngQout_cms, dy_m)
    End Sub


    Private Sub CalReservoirOutFlowInReservoirOperation(ByVal cv As cCVAttribute, ByVal sngQout_cms As Single, ByVal sngDY_m As Single)
        With cv
            If sngQout_cms > 0 Then
                If .FlowType = cGRM.CellFlowType.OverlandFlow Then
                    .QCVof_i_j_m3Ps = sngQout_cms
                    .hCVof_i_j = CSng((.RoughnessCoeffOF * .QCVof_i_j_m3Ps / sngDY_m) / (.SlopeOF ^ 0.5) ^ 0.6)

                    .mStreamAttr.QCVch_i_j_m3Ps = 0
                    .mStreamAttr.uCVch_i_j = 0
                    .mStreamAttr.CSAch_i_j = 0
                    .mStreamAttr.hCVch_i_j = 0
                    .mStreamAttr.uCVch_i_j = 0
                Else
                    .QCVof_i_j_m3Ps = 0
                    .hCVof_i_j = 0
                    .mStreamAttr.QCVch_i_j_m3Ps = sngQout_cms
                    .mStreamAttr.CSAch_i_j = mFVMsolver.CalChCSAFromQbyIteration(cv, .mStreamAttr.CSAch_i_j, .mStreamAttr.QCVch_i_j_m3Ps)
                    .mStreamAttr.uCVch_i_j = .mStreamAttr.QCVch_i_j_m3Ps / .mStreamAttr.CSAch_i_j
                    .mStreamAttr.hCVch_i_j = mFVMsolver.GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth, .mStreamAttr.CSAch_i_j,
                                                             .mStreamAttr.chIsCompoundCS, .mStreamAttr.chUpperRBaseWidth_m,
                                                             .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chLowerRHeight,
                                                             .mStreamAttr.mChBankCoeff)
                End If
            Else
                .hCVof_i_j = 0
                .CSAof_i_j = 0
                .QCVof_i_j_m3Ps = 0
                .mStreamAttr.uCVch_i_j = 0
                .mStreamAttr.CSAch_i_j = 0
                .mStreamAttr.hCVch_i_j = 0
                .mStreamAttr.uCVch_i_j = 0
                .mStreamAttr.QCVch_i_j_m3Ps = 0
            End If
        End With
    End Sub


    Public Sub CalFCReservoirOutFlow(ByVal project As cProject, ByVal nowT_MIN As Integer, ByVal cvan As Integer)
        Dim fcCVid As Integer = project.CV(cvan).CVID
        Dim Rows() As DataRow = project.FCGrid.mdtFCGridInfo.Select(String.Format("cvid = {0}", fcCVid))
        Dim fcRow As GRMProject.FlowControlGridRow = CType(Rows(0), GRMProject.FlowControlGridRow)
        '60 /60의 경우에는 intFCDataArrayNum=0을 써야 하지만 나눈 몫이 1이므로, 배열 번호가 넘어가게 된다.
        '따라서, 59/60 을 만들기 위해서 (intNowTimeMin - 1)으로 한다.
        Dim rowOrder As Integer = CInt((nowT_MIN - 1) \ CInt(fcRow.DT))
        Dim fcDataRows() As DataRow = project.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0}", fcCVid))
        If rowOrder >= fcDataRows.Length Then rowOrder = fcDataRows.Length - 1
        With project.CV(cvan)
            If .FlowType = cGRM.CellFlowType.OverlandFlow Then
                System.Console.WriteLine("ERROR: Reservoir outflow is simulated only in channel flow!!!    ")
            Else
                If fcDataRows(rowOrder).Item("value").ToString = "-" Then   '2018.9.3 원 : 추가. 이는 임시조치임. 최박사님과 조치 예정
                    .mStreamAttr.QCVch_i_j_m3Ps = 0
                Else
                    .mStreamAttr.QCVch_i_j_m3Ps = CSng(fcDataRows(rowOrder).Item("value"))
                End If

                .mStreamAttr.CSAch_i_j = mFVMsolver.CalChCSAFromQbyIteration(project.CV(cvan), .mStreamAttr.CSAch_i_j, .mStreamAttr.QCVch_i_j_m3Ps)
                'Dim csa1 As Single = .mStreamAttr.CSAch_i_j
                'Dim chCSAini2 As Single = mFVMsolver.CalChCSAFromManningEQ(project.CV(cvan), .mStreamAttr.CSAch_i_j, .mStreamAttr.QCVch_i_j_m3Ps)
                .mStreamAttr.hCVch_i_j = mFVMsolver.GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth, .mStreamAttr.CSAch_i_j,
                                                         .mStreamAttr.chIsCompoundCS, .mStreamAttr.chUpperRBaseWidth_m,
                                                         .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chLowerRHeight, .mStreamAttr.mChBankCoeff)
                .mStreamAttr.uCVch_i_j = .mStreamAttr.QCVch_i_j_m3Ps / .mStreamAttr.CSAch_i_j
            End If
            project.FCGrid.mFCdataToApplyNowT(fcCVid) = .mStreamAttr.QCVch_i_j_m3Ps   '2018.9.3 원 : 다시 fcDataRows(rowOrder).Item("value") 가져오는거 보다 낳다고 판단했슴 
        End With
    End Sub


    Public Sub CalFCSinkOrSourceFlow(ByVal project As cProject, ByVal nowT_MIN As Integer, ByVal cvan As Integer)
        Dim sngCellSize As Single = project.Watershed.mCellSize
        Dim fcCVid As Integer = project.CV(cvan).CVID
        Dim Rows() As DataRow = project.FCGrid.mdtFCGridInfo.Select(String.Format("cvid = {0}", fcCVid))
        Dim fcRow As GRMProject.FlowControlGridRow = CType(Rows(0), GRMProject.FlowControlGridRow)
        Dim rowOrder As Integer = CInt((nowT_MIN - 1) \ CInt(fcRow.DT))
        Dim dv As DataView
        dv = New DataView(project.FCGrid.mdtFCFlowData, String.Format("cvid={0}", fcCVid), " datetime asc", DataViewRowState.CurrentRows)
        Dim dt As DataTable = dv.Table
        Dim fcdt As DataTable = CType(dt, DataTable)
        If rowOrder < fcdt.Rows.Count Then
            Dim QtoApp As Single = CSng(fcdt.Rows(rowOrder).Item("value"))
            With project.CV(cvan)
                If .FlowType = cGRM.CellFlowType.OverlandFlow Then
                    Select Case .FCType
                        Case cFlowControl.FlowControlType.SinkFlow
                            .QCVof_i_j_m3Ps = .QCVof_i_j_m3Ps - QtoApp
                        Case cFlowControl.FlowControlType.SourceFlow
                            .QCVof_i_j_m3Ps = .QCVof_i_j_m3Ps + QtoApp
                    End Select
                    If .QCVof_i_j_m3Ps < 0 Then
                        .QCVof_i_j_m3Ps = 0
                    End If
                    .hCVof_i_j = CSng(((.RoughnessCoeffOF * .QCVof_i_j_m3Ps / sngCellSize) / (.SlopeOF ^ 0.5)) ^ 0.6)
                    .CSAof_i_j = .hCVof_i_j * sngCellSize
                    .mStreamAttr.uCVch_i_j = .QCVof_i_j_m3Ps / .CSAof_i_j
                Else
                    Select Case .FCType
                        Case cFlowControl.FlowControlType.SinkFlow
                            .mStreamAttr.QCVch_i_j_m3Ps = .mStreamAttr.QCVch_i_j_m3Ps - QtoApp
                        Case cFlowControl.FlowControlType.SourceFlow
                            .mStreamAttr.QCVch_i_j_m3Ps = .mStreamAttr.QCVch_i_j_m3Ps + QtoApp
                    End Select
                    If .mStreamAttr.QCVch_i_j_m3Ps < 0 Then .mStreamAttr.QCVch_i_j_m3Ps = 0
                    .mStreamAttr.CSAch_i_j = mFVMsolver.CalChCSAFromQbyIteration(project.CV(cvan), .mStreamAttr.CSAch_i_j, .mStreamAttr.QCVch_i_j_m3Ps)
                    .mStreamAttr.hCVch_i_j = mFVMsolver.GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth, .mStreamAttr.CSAch_i_j,
                                                            .mStreamAttr.chIsCompoundCS, .mStreamAttr.chUpperRBaseWidth_m,
                                                            .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chLowerRHeight, .mStreamAttr.mChBankCoeff)
                    .mStreamAttr.uCVch_i_j = .mStreamAttr.QCVch_i_j_m3Ps / .mStreamAttr.CSAch_i_j
                End If
                project.FCGrid.mFCdataToApplyNowT(fcCVid) = CDbl(QtoApp)
            End With
        Else
            project.FCGrid.mFCdataToApplyNowT(fcCVid) = 0
        End If
    End Sub

End Class
