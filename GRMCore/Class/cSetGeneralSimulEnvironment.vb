Option Strict Off
Public Class cSetGeneralSimulEnvironment

#Region "Simulation Tab"

    Public mIsDateTimeFormat As Boolean = False
    Public mSimStartDateTime As String = Nothing
    ''' <summary>
    ''' 모의 기간[hour]
    ''' </summary>
    ''' <remarks></remarks>
    Public mSimDurationHOUR As Nullable(Of Integer)
    ''' <summary>
    ''' Printout time step[minute]
    ''' </summary>
    ''' <remarks></remarks>
    Public mPrintOutTimeStepMIN As Nullable(Of Integer)

#End Region

#Region "모의 옵션"
    Public mbSimulateInfiltration As Boolean = True
    Public mbSimulateSSFlow As Boolean = True
    Public mbSimulateFlowControl As Boolean = False
    Public mbSimulateBFlow As Boolean = True

    Public mbEnableAnalyzer As Boolean = False
    Public mbShowSoilSaturation As Boolean = False
    Public mbShowRFdistribution As Boolean = False
    Public mbShowRFaccDistribution As Boolean = False
    Public mbShowFlowDistribution As Boolean = False
    Public mbCreateImageFile As Boolean = False
    Public mbCreateASCFile As Boolean = False

    Public mPrintOption As cGRM.GRMPrintType
#End Region

#Region "모델링 환경"
    Public mAboutThisProject As String
    Public mAboutWatershed As String
    Public mAboutLandCoverMap As String
    Public mAboutSoilMap As String
    Public mAboutSoilDepthMap As String
    Public mAboutRainfall As String
#End Region

    ''' <summary>
    ''' 모델링 종료시간을 출력하기 위한 문자형 변수
    ''' </summary>
    ''' <remarks></remarks>
    Public mEndingTimeToPrint As String

    Public Sub New()
        mSimDurationHOUR = Nothing
        mPrintOutTimeStepMIN = Nothing
    End Sub

    Public Sub GetValues(ByVal prjDB As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        If Not row.IsSimulationDurationNull Then
            With row

                If Not .IsSimulStartingTimeNull Then
                    mSimStartDateTime = .SimulStartingTime
                Else
                    mSimStartDateTime = "0"
                End If
                mIsDateTimeFormat = Not IsNumeric(mSimStartDateTime)
                mSimDurationHOUR = .SimulationDuration
                mPrintOutTimeStepMIN = .OutputTimeStep
                cThisSimulation.dtMaxLimit_sec = mPrintOutTimeStepMIN / 2 * 60
                mbSimulateBFlow = .SimulateBaseFlow
                mbSimulateSSFlow = .SimulateSubsurfaceFlow
                mbSimulateFlowControl = .SimulateFlowControl
                mbSimulateInfiltration = .SimulateInfiltration

                mbCreateASCFile = False
                If Not .IsMakeASCFileNull AndAlso .MakeASCFile.ToLower = "true" Then
                    mbCreateASCFile = True
                End If

                mbCreateImageFile = False
                If Not .IsMakeIMGFileNull AndAlso .MakeIMGFile.ToLower = "true" Then
                    mbCreateImageFile = True
                End If

                mbShowFlowDistribution = False
                If Not .IsMakeFlowDistFileNull AndAlso .MakeFlowDistFile.ToLower = "true" Then
                    mbShowFlowDistribution = .MakeFlowDistFile
                End If

                mbShowRFaccDistribution = False
                If Not .IsMakeRFaccDistFileNull AndAlso .MakeRFaccDistFile.ToLower = "true" Then
                    mbShowRFaccDistribution = True
                End If

                mbShowRFdistribution = False
                If Not .IsMakeRfDistFileNull AndAlso .MakeRfDistFile.ToLower = "true" Then
                    mbShowRFdistribution = True
                End If

                mbShowSoilSaturation = False
                If Not .IsMakeSoilSaturationDistFileNull AndAlso .MakeSoilSaturationDistFile.ToLower = "true" Then
                    mbShowSoilSaturation = True
                End If

                If mbCreateImageFile = True OrElse mbCreateASCFile = True Then
                    mbEnableAnalyzer = True
                Else
                    mbEnableAnalyzer = False
                End If
                If Not .IsPrintOptionNull Then
                    Select Case .PrintOption.ToString.ToLower
                        Case cGRM.GRMPrintType.AllQ.ToString.ToLower
                            mPrintOption = cGRM.GRMPrintType.AllQ
                        Case cGRM.GRMPrintType.DischargeFileQ.ToString.ToLower
                            mPrintOption = cGRM.GRMPrintType.DischargeFileQ
                        Case Else
                            mPrintOption = cGRM.GRMPrintType.All
                    End Select
                Else
                    mPrintOption = cGRM.GRMPrintType.All
                End If
                If Not .IsAboutThisProjectNull Then mAboutThisProject = .AboutThisProject
                If Not .IsAboutWatershedNull Then mAboutWatershed = .AboutWatershed
                If Not .IsAboutLandCoverMapNull Then mAboutLandCoverMap = .AboutLandCoverMap
                If Not .IsAboutSoilMapNull Then mAboutSoilMap = .AboutSoilMap
                If Not .IsAboutSoilDepthMapNull Then mAboutSoilDepthMap = .AboutSoilDepthMap
                If Not .IsAboutRainfallNull Then mAboutRainfall = .AboutRainfall
            End With
        End If
    End Sub

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mSimDurationHOUR.HasValue
        End Get
    End Property

    Public Sub SetValues(ByVal prjDB As GRMProject)
        If mSimDurationHOUR IsNot Nothing Then
            Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With row
                .SimulationDuration = mSimDurationHOUR.Value
                .OutputTimeStep = mPrintOutTimeStepMIN.Value
                .SimulateBaseFlow = mbSimulateBFlow
                .SimulateSubsurfaceFlow = mbSimulateSSFlow
                .SimulateFlowControl = mbSimulateFlowControl
                .SimulateInfiltration = mbSimulateInfiltration
                .MakeASCFile = mbCreateASCFile
                .MakeIMGFile = mbCreateImageFile
                .PrintOption = mPrintOption.ToString
                If mAboutThisProject <> "" Then .AboutThisProject = mAboutThisProject
                If mAboutWatershed <> "" Then .AboutWatershed = mAboutWatershed
                If mAboutLandCoverMap <> "" Then .AboutLandCoverMap = mAboutLandCoverMap
                If mAboutSoilMap <> "" Then .AboutSoilMap = mAboutSoilMap
                If mAboutSoilDepthMap <> "" Then .AboutSoilDepthMap = mAboutSoilDepthMap
                If mAboutRainfall <> "" Then .AboutRainfall = mAboutRainfall
                .ProjectSavedTime = Format(Date.Now, "yyyy/MM/dd HH:mm")
                .ComputerName = Environment.MachineName
                .ComputerUserName = Environment.UserName
                .GRMVersion = cGRM.BuildInfo.ProductVersion
            End With
        End If
    End Sub

    ''' <summary>
    ''' 모델링 종료시간을 출력하기 위한 문자형 변수
    ''' </summary>
    ''' <remarks></remarks>
    Public ReadOnly Property EndingTimeToPrint() As String
        Get
            Return mEndingTimeToPrint
        End Get
    End Property


    ''' <summary>
    ''' 모델링 기간과 출력 시간간격을 이용해서 모델링 끝나는 시간을 계산[sec]
    ''' </summary>
    ''' <remarks>'이때 사용자가 지정한 기간보다 한시간 더 모의한다.
    ''' 왜냐하면, 시작시 0으로 출력되는 것이 실제로는 1시간 모의가 되므로, 
    ''' 10시간 입력시, 실제로는 9시간이 마지막 출력이 된다.
    ''' 그러므로 혼동을 없애기 위해 사용자가 지정한 시간으로 모의결과를 마치기 위해서 1시간을 더 모의해 준다. </remarks>
    Public ReadOnly Property EndingTimeSec() As Integer
        Get
            Return CInt((mSimDurationHOUR * 60 + mPrintOutTimeStepMIN) * 60)
        End Get
    End Property


    Private Function GetProductVersion(ByVal strInstalledProductName As String) As String
        Try
            Dim oInstaller As Object
            Dim Y As Object
            oInstaller = CreateObject("WindowsInstaller.Installer")
            GetProductVersion = ""
            For Each Y In oInstaller.Products
                If LCase(strInstalledProductName) = LCase(oInstaller.ProductInfo(Y, "InstalledProductName")) Then
                    GetProductVersion = oInstaller.ProductInfo(Y, "VersionString")
                    Exit Function
                End If
            Next
        Catch ex As Exception
            System.Console.WriteLine(ex.ToString)
            GetProductVersion = ""
        End Try
    End Function

End Class
