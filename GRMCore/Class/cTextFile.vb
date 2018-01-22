Imports System.Text

Public Class cTextFile
    Public Enum ValueSeparator
        CSV
        SPACE
        TAB
        ALL
    End Enum
    Public Shared Function CreateASCTextFile(ByVal fpn As String, ByVal ncols As Integer, ByVal nrows As Integer, _
                                                           ByVal xll As Double, ByVal yll As Double, _
                                                           ByVal cellSize As Single, ByVal nodataValue As String, ByVal strArray As String()) As Boolean

        'If File.Exists(fpn) Then File.Delete(fpn)
        IO.File.AppendAllText(fpn, "ncols" + "    " + CStr(ncols) + vbCrLf)
        IO.File.AppendAllText(fpn, "nrows" + "    " + CStr(nrows) + vbCrLf)
        IO.File.AppendAllText(fpn, "xllcorner" + "    " + CStr(xll) + vbCrLf)
        IO.File.AppendAllText(fpn, "yllcorner" + "    " + CStr(yll) + vbCrLf)
        IO.File.AppendAllText(fpn, "cellsize" + "    " + CStr(cellSize) + vbCrLf)
        IO.File.AppendAllText(fpn, "NODATA_value" + "    " + nodataValue + vbCrLf)
        For n As Integer = 0 To strArray.Length - 1
            IO.File.AppendAllText(fpn, strArray(n) + vbCrLf)
        Next
        Return True
    End Function

    Public Shared Function ReadTextFileAndSetDataTable(FPNsource As String, valueSeparator As cTextFile.ValueSeparator, Optional fieldCount As Integer = 0) As DataTable
        Try
            If FPNsource = "" OrElse File.Exists(FPNsource) = False Then
                MsgBox("Source text file and data is invalid.   ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
                Return Nothing
            End If

            Dim intL As Integer = 0
            Dim dt As New DataTable
            Using oTextReader As New FileIO.TextFieldParser(FPNsource, Encoding.Default)
                oTextReader.TextFieldType = FileIO.FieldType.Delimited
                oTextReader.SetDelimiters(GetTextFileValueSeparator(valueSeparator))
                oTextReader.TrimWhiteSpace = True
                Dim TextIncurrentRow As String()
                dt.BeginLoadData()
                While Not oTextReader.EndOfData
                    TextIncurrentRow = oTextReader.ReadFields
                    If Trim(TextIncurrentRow(0)).ToString = "" Then
                        MsgBox(String.Format("{0} line has empty value. Exit reading text file process.", intL + 1))
                        Exit While
                    End If
                    Dim nFieldCount As Integer = TextIncurrentRow.Length
                    If intL = 0 Then
                        If fieldCount > 0 AndAlso nFieldCount <> fieldCount Then
                            '여기서 설정된 필드 개수와 읽은 시계열 개수 검증
                            MsgBox("Data series number in text file is different with user settings.   ", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                            Return Nothing
                        End If
                        For Each ele As String In TextIncurrentRow
                            dt.Columns.Add(ele.ToString)
                        Next
                    Else
                        Dim nr As DataRow = dt.NewRow
                        For nG1 As Integer = 0 To nFieldCount - 1
                            nr.Item(nG1) = TextIncurrentRow(nG1).ToString
                        Next nG1
                        dt.Rows.Add(nr)
                    End If
                    intL += 1
                End While
                dt.EndLoadData()
            End Using
            Return dt
        Catch ex As Exception
            Throw ex
        End Try
    End Function



    Public Shared Function MakeTextFileUsingDataTable(dtInput As DataTable, valueSeparator As cTextFile.ValueSeparator,
                                                      fpnTarget As String, bWriteColumnHeader As Boolean, WriteAsIntger As Boolean) As Boolean
        Dim stb As New StringBuilder

        If bWriteColumnHeader = True Then
            For Each col As DataColumn In dtInput.Columns
                stb.Append(col.ColumnName.ToString.Trim & ",")
            Next
            stb.Replace(",", vbCrLf, stb.Length - 1, 1)
        End If
        For Each dr As DataRow In dtInput.Rows
            Dim stbAline As New StringBuilder
            For Each field As Object In dr.ItemArray
                If WriteAsIntger = True Then
                    Dim value As Integer
                    Dim valuestr As String
                    If Integer.TryParse(field.ToString, value) = True Then
                        valuestr = value.ToString
                    Else
                        valuestr = field.ToString
                    End If
                    stbAline.Append(valuestr.Trim & ",") '개수가 엄청 많으므로.. 정수형으로 기록한다.
                Else
                    stbAline.Append(field.ToString.Trim & ",")
                End If
            Next
            stbAline.Replace(",", vbCrLf, stbAline.Length - 1, 1)
            stb.Append(stbAline)
        Next
        Try
            File.WriteAllText(fpnTarget, stb.ToString, Encoding.Default)
            Return True
        Catch ex As Exception
            Throw ex
            Return False
        End Try
    End Function


    Public Shared Function ReadGRMoutFileAndMakeStringArray(BaseString As String(), FPNsource As String,
                                                          rowNmuberToBeginRead As Integer, columnNumberToRead As Integer, columnName As String,
                                                          ValueSeparatorInSourceFile As String(), valueSeparatorInReturnArray As String,
                                                          valueAsInteger As Boolean) As String()
        If FPNsource = "" OrElse File.Exists(FPNsource) = False Then
            MsgBox("Source text file and data is invalid.   ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
            Return Nothing
        End If

        'Dim sepArray As String() = GetTextFileValueSeparator(ValueSeparatorInSourceFile)
        Dim Lines() As String = System.IO.File.ReadAllLines(FPNsource)
        Dim nr As Integer = 0
        '이건 해더 기록
        If BaseString(0) Is Nothing OrElse BaseString(0).ToString.Trim = "" Then
            BaseString(nr) = columnName
        Else
            BaseString(0) = BaseString(0) + valueSeparatorInReturnArray + columnName
        End If
        For nl As Integer = 0 To Lines.Length - 1
            If nl >= rowNmuberToBeginRead - 1 Then
                nr += 1
                If (nr + 1) > BaseString.Length Then
                    MsgBox("Input file contains more row count than base string array.   ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
                    Exit For
                End If
                Dim aLine As String = Lines(nl)
                Dim parts() As String = aLine.Split(ValueSeparatorInSourceFile, StringSplitOptions.RemoveEmptyEntries)
                Dim value As String
                Dim stbAline As New StringBuilder
                If valueAsInteger = True Then
                    value = CInt(parts(columnNumberToRead - 1)).ToString.Trim
                Else
                    value = parts(columnNumberToRead - 1).ToString.Trim '데이터 양이 많을 경우, 정수형으로 쓴다.
                End If
                If BaseString(nr) Is Nothing OrElse BaseString(nr).ToString.Trim = "" Then
                    BaseString(nr) = value
                Else
                    BaseString(nr) = BaseString(nr) + valueSeparatorInReturnArray + value
                End If

            End If
        Next
        Return BaseString
    End Function

    Public Shared Function ReadGRMoutFileAndFillUpDataTable(dtInput As DataTable, FPNsource As String, _
                                               valueSeparator As cTextFile.ValueSeparator, _
                                               columeNumberToRead As Integer, columeNameToFill As String, _
                                               rowNmuberToBeginRead As Integer, valueAsInteger As Boolean) As DataTable
        Try
            If FPNsource = "" OrElse File.Exists(FPNsource) = False Then
                MsgBox("Source text file and data is invalid.   ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
                Return Nothing
            End If
            If columeNumberToRead < 1 Then
                MsgBox("Invalid colume number to read is set.   ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
                Return Nothing
            End If

            Dim sepArray As String() = GetTextFileValueSeparator(valueSeparator)
            Dim Lines() As String = System.IO.File.ReadAllLines(FPNsource)
            Dim nr As Integer = 0
            dtInput.BeginLoadData()
            For nl As Integer = 0 To Lines.Length - 1
                If nl >= rowNmuberToBeginRead - 1 Then
                    If (nr + 1) > dtInput.Rows.Count Then
                        MsgBox("Input file contains more row count than time data count.   ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
                        Exit For
                    End If
                    Dim aLine As String = Lines(nl)
                    Dim parts() As String = aLine.Split(sepArray, StringSplitOptions.RemoveEmptyEntries)
                    If valueAsInteger = True Then
                        dtInput.Rows(nr).Item(columeNameToFill) = CInt(parts(columeNumberToRead - 1)).ToString.Trim
                    Else
                        dtInput.Rows(nr).Item(columeNameToFill) = parts(columeNumberToRead - 1).ToString.Trim '데이터 양이 많을 경우, 정수형으로 쓴다.
                    End If
                    nr += 1
                End If
            Next
            dtInput.EndLoadData()
            Return dtInput
        Catch ex As Exception
            Throw ex
        End Try
    End Function

    Private Shared Function GetTextFileValueSeparator(valueSeparator As cTextFile.ValueSeparator) As String()
        Dim sepArray As String()
        Select Case valueSeparator
            Case cTextFile.ValueSeparator.CSV
                sepArray = {","}
            Case cTextFile.ValueSeparator.SPACE
                sepArray = {" "}
            Case cTextFile.ValueSeparator.TAB
                sepArray = {vbTab}
            Case cTextFile.ValueSeparator.ALL
                sepArray = {",", " ", vbTab}
            Case Else
                sepArray = {",", " ", vbTab}
        End Select
        Return sepArray
    End Function

    Public Shared Function GetValueStartingRowNumber(fpnSource As String, valueSeparator As cTextFile.ValueSeparator) As Integer
        Dim Lines() As String = System.IO.File.ReadAllLines(fpnSource)
        Dim nr As Integer = 0
        Dim sepArray As String() = GetTextFileValueSeparator(valueSeparator)
        For nl As Integer = 0 To Lines.Length - 1
            Dim aLine As String = Lines(nl)
            Dim parts() As String = aLine.Split(sepArray, StringSplitOptions.RemoveEmptyEntries)
            If parts.Length > 1 Then
                If parts(0) = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME Then
                    nr = nl + 2
                    Exit For
                End If
            End If
        Next
        Return nr
    End Function


    ''' <summary>
    ''' VAT file을 읽어서 SortedList로 반환
    ''' </summary>
    ''' <param name="filePath"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Shared Function ReadVatFile(ByVal filePath As String) As SortedList(Of Integer, String)
        Dim values As New SortedList(Of Integer, String)
        Dim reader As New StreamReader(filePath, System.Text.Encoding.Default)
        ' 첫 행은 전체 행수, 여기서 무시
        reader.ReadLine()

        ' VAT 값 읽어서 넣기
        While Not reader.EndOfStream
            Dim line As String = reader.ReadLine
            Dim parts() As String = line.Split(New String() {","}, StringSplitOptions.RemoveEmptyEntries)
            Dim attrValue As Integer
            If parts IsNot Nothing AndAlso parts.Length = 2 AndAlso _
                Integer.TryParse(parts(0), attrValue) AndAlso _
                Not values.ContainsKey(attrValue) Then
                values.Add(attrValue, parts(1))
            End If
        End While
        reader.Close()
        Return values
    End Function


    ''' <summary>
    ''' 이건 수정할 라인의 번호를 미리 알고 있거나, 포함된 문자 중 일부를 알고 있을때 사용
    ''' </summary>
    ''' <param name="strSourceFPN"></param>
    ''' <param name="strTagetFPN"></param>
    ''' <param name="TagetLine"></param>
    ''' <param name="strTextToReplace"></param>
    ''' <remarks></remarks>
    Public Shared Sub ReplaceALineInTextFile(ByVal strSourceFPN As String, ByVal strTagetFPN As String, ByVal TagetLine As Integer, ByVal strTextToReplace As String)
        Try
            Dim Lines() As String = System.IO.File.ReadAllLines(strSourceFPN)
            Lines(TagetLine - 1) = strTextToReplace
            System.IO.File.WriteAllLines(strTagetFPN, Lines)
        Catch ex As Exception
            Throw
        End Try
    End Sub

    ''' <summary>
    ''' 이건 수정할 라인의 번호를 미리 알고 있거나, 포함된 문자 중 일부를 알고 있을때 사용
    ''' </summary>
    ''' <param name="strSourceFNP"></param>
    ''' <param name="strTagetFNP"></param>
    ''' <param name="ContainedTextInALine"></param>
    ''' <param name="strTextToReplace"></param>
    ''' <remarks></remarks>
    Public Shared Sub ReplaceALineInTextFile(ByVal strSourceFNP As String, ByVal strTagetFNP As String, ByVal ContainedTextInALine As String, ByVal strTextToReplace As String)
        Try
            Dim Lines() As String = System.IO.File.ReadAllLines(strSourceFNP)
            For n As Integer = 0 To Lines.Length - 1
                If Lines(n).Contains(ContainedTextInALine) Then
                    Lines(n) = strTextToReplace
                End If
            Next
            System.IO.File.WriteAllLines(strTagetFNP, Lines)
        Catch ex As Exception
            Throw
        End Try
    End Sub

    Public Shared Function ReplaceALineInStringArray(ByVal sourceArray As String(), textToFindInALine As String, ByVal textToReplace As String) As String()
        Try
            For n As Integer = 0 To sourceArray.Length - 1
                If sourceArray(n).Contains(textToFindInALine) Then
                    sourceArray(n) = textToReplace
                End If
            Next
            Return sourceArray
        Catch ex As Exception
            Throw
        End Try
    End Function

    Public Shared Sub ReplaceTextInTextFile(ByVal strSourceFNP As String, ByVal strTagetFNP As String, ByVal strTextToFind As String, ByVal strTextToReplace As String)
        Try
            Dim strLines() As String = System.IO.File.ReadAllLines(strSourceFNP)
            Dim intTotCountLine As Integer = strLines.Length
            Dim intNLine As Integer
            Dim strOneLine As String
            For intNLine = 0 To intTotCountLine - 1
                strOneLine = CStr(strLines(intNLine))
                strLines(intNLine) = Replace(strOneLine, strTextToFind, strTextToReplace)
            Next
            System.IO.File.WriteAllLines(strTagetFNP, strLines)
        Catch ex As Exception
            Throw
        End Try
    End Sub

    Public Shared Sub ReplaceTextInTextFile(ByVal strSourceFNP As String, ByVal strTextToFind As String, ByVal strTextToReplace As String)
        Try
            Dim strLines() As String = System.IO.File.ReadAllLines(strSourceFNP)
            Dim intTotCountLine As Integer = strLines.Length
            Dim intNLine As Integer
            Dim strOneLine As String
            For intNLine = 0 To intTotCountLine - 1
                strOneLine = CStr(strLines(intNLine))
                strLines(intNLine) = Replace(strOneLine, strTextToFind, strTextToReplace)
            Next
            System.IO.File.WriteAllLines(strSourceFNP, strLines)
        Catch ex As Exception
            Throw
        End Try
    End Sub

End Class
