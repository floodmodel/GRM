'Imports System.Math
Public Class cHydroCom

    Public Shared Function GetFlowDirection(value As Integer, FDType As cGRM.FlowDirectionType) As cGRM.GRMFlowDirectionD8
        If FDType = cGRM.FlowDirectionType.StartsFromNE Then
            Select Case value
                Case 1
                    Return cGRM.GRMFlowDirectionD8.NE
                Case 2
                    Return cGRM.GRMFlowDirectionD8.E
                Case 4
                    Return cGRM.GRMFlowDirectionD8.SE
                Case 8
                    Return cGRM.GRMFlowDirectionD8.S
                Case 16
                    Return cGRM.GRMFlowDirectionD8.SW
                Case 32
                    Return cGRM.GRMFlowDirectionD8.W
                Case 64
                    Return cGRM.GRMFlowDirectionD8.NW
                Case 128
                    Return cGRM.GRMFlowDirectionD8.N
            End Select
        End If
        If FDType = cGRM.FlowDirectionType.StartsFromN Then
            Select Case value
                Case 1
                    Return cGRM.GRMFlowDirectionD8.N
                Case 2
                    Return cGRM.GRMFlowDirectionD8.NE
                Case 4
                    Return cGRM.GRMFlowDirectionD8.E
                Case 8
                    Return cGRM.GRMFlowDirectionD8.SE
                Case 16
                    Return cGRM.GRMFlowDirectionD8.S
                Case 32
                    Return cGRM.GRMFlowDirectionD8.SW
                Case 64
                    Return cGRM.GRMFlowDirectionD8.W
                Case 128
                    Return cGRM.GRMFlowDirectionD8.NW
            End Select
        End If
        If FDType = cGRM.FlowDirectionType.StartsFromE Then
            Select Case value
                Case 1
                    Return cGRM.GRMFlowDirectionD8.E
                Case 2
                    Return cGRM.GRMFlowDirectionD8.SE
                Case 4
                    Return cGRM.GRMFlowDirectionD8.S
                Case 8
                    Return cGRM.GRMFlowDirectionD8.SW
                Case 16
                    Return cGRM.GRMFlowDirectionD8.W
                Case 32
                    Return cGRM.GRMFlowDirectionD8.NW
                Case 64
                    Return cGRM.GRMFlowDirectionD8.N
                Case 128
                    Return cGRM.GRMFlowDirectionD8.NE
            End Select
        End If

        If FDType = cGRM.FlowDirectionType.StartsFromE_TauDEM Then
            Select Case value
                Case 1
                    Return cGRM.GRMFlowDirectionD8.E
                Case 2
                    Return cGRM.GRMFlowDirectionD8.NE
                Case 3
                    Return cGRM.GRMFlowDirectionD8.N
                Case 4
                    Return cGRM.GRMFlowDirectionD8.NW
                Case 5
                    Return cGRM.GRMFlowDirectionD8.W
                Case 6
                    Return cGRM.GRMFlowDirectionD8.SW
                Case 7
                    Return cGRM.GRMFlowDirectionD8.S
                Case 8
                    Return cGRM.GRMFlowDirectionD8.SE
            End Select
        End If

    End Function


    Public Shared Function getDTsec(cfln As Single, dx As Integer,
                                    vMax As Single, dtPrint_min As Integer) As Integer
        If vMax <= 0 Then
            Return cThisSimulation.dtMaxLimit_sec
        End If
        Dim dtsecNext As Double = cGRM.CONST_CFL_NUMBER * dx / vMax
        dtsecNext = CInt(Math.Truncate(dtsecNext))

        If dtsecNext > cThisSimulation.dtMaxLimit_sec Then
            dtsecNext = cThisSimulation.dtMaxLimit_sec
        End If
        If dtsecNext < cThisSimulation.dtMinLimit_sec Then
            dtsecNext = cThisSimulation.dtMinLimit_sec
        End If

        If dtsecNext > (dtPrint_min * 60 / 2) Then '출력시간간격의 1/2 제한을 둔다.. 즉, 출력하기 전에 두번은 계산하도록
            dtsecNext = dtPrint_min * 60 / 2
        End If

        Return CInt(dtsecNext)
    End Function



    ' ''' <summary>
    ' ''' 격자크기 조건을 이용해서 계산 시간간격(dt) 자동 계산
    ' ''' 2015.03.26.  5분을 defalut 로 설정
    ' ''' </summary>
    ' ''' <param name="intDeltaX_meter"></param>
    ' ''' <returns></returns>
    ' ''' <remarks>
    ' ''' '계산 시간간격은 셀사이즈에 따라서 결정한다. </remarks>
    '셀크기[m] 지표수심 유속	dtsec   dtmin	적용dtsec
    '   10	    200	    0.72 	13.87 	0.23 	5
    '   20	    200	    0.72 	27.74 	0.46 	10
    '   30	    200	    0.72 	41.61 	0.69 	20
    '   60	    200	    0.72 	83.22 	1.39 	60
    '   90	    200	    0.72 	124.83 	2.08 	60
    '  100	    200	    0.72 	138.70 	2.31 	120
    '  120	    200	    0.72 	166.44 	2.77 	120
    '  150	    200	    0.72 	208.05 	3.47 	180
    '  200	    200	    0.72 	277.40 	4.62 	180
    '  500	    200	    0.72 	693.49 	11.56 	300
    ' 1000	    200	    0.72 	1386.98 23.12 	600
    ' 
    '   m	m/s	s	s	
    '  셀크기	유속  dt	 적용DT	분
    '  50 4	12.5  12	0.2
    ' 100	4	25	  30	0.5
    ' 150	4	37.5  30	0.5
    ' 200	4	50	  30	0.5
    ' 250	4	62.5  60	1
    ' 300	4	75	  60	1
    ' 500	4	125	 120	2
    ' 600	4	150	 120	2
    ' 900	4	225	 180	3
    '1000	4	250	 180	3
    '1200	4	300	 300	5
    '1500	4	375	 300	5
    '2000	4	500	 300	5
    '2500	4	625	 600	10
    '3000	4	750	 600	10

    ''' <summary>
    ''' 기지의 x축의 두점 t1, t2에 대한 기지의 y 축 값 A, B를 이용해서 대상 x 축 값 tx에 대한 미지의 y축 값 X를 
    ''' 계산하기 위한 함수. 이때 t2는 t1 보다 큰 값임.
    ''' 기지의 두점 사이는 직선으로 가정하고, 그 기울기를 이용해서, 미지값을 보간하여 계산함
    ''' </summary>
    ''' <param name="A">t1에 대한 y축 값</param>
    ''' <param name="B">t2에 대한 y축 값</param>
    ''' <param name="interCoef"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Shared Function GetInterpolatedValueLinear(ByVal A As Single, ByVal B As Single,
                                                      ByVal interCoef As Single) As Single
        Try
            Dim X As Single
            X = (B - A) * interCoef + A
            Return X
        Catch ex As Exception
            Return Single.MinValue
        End Try

    End Function
End Class

