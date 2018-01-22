Imports MapWindow.Interfaces
Imports MapWinGIS
Public Class cMap
    Public Shared mwAppMain As MapWindow.Interfaces.IMapWin
    'Public Shared mwMap As New AxMapWinGIS.AxMap ' 이건 grm에서 사용하는 mapwindow 창

    Public Shared Function GetMWLayerHandleWithLayerName _
        (map As MapWindow.Interfaces.IMapWin, lyrName As String) As Integer
        For Each lyr As Layer In map.Layers
            If Trim(lyr.Name) = lyrName Then
                Return lyr.Handle
            End If
        Next
        Return Nothing
    End Function


    'Public Shared Function GetMWLayerHandleWithLayerName _
    '    (map As AxMapWinGIS.AxMap, lyrName As String) As Integer
    '    Dim mwUtil As New MapWinGIS.Utils
    '    For n As Integer = 0 To map.NumLayers - 1
    '        Dim hndle As Integer = map.get_LayerHandle(n)

    '        Dim lyr As Layer = CType(map.get_GetObject(hndle), Layer)
    '        If lyr.Name = lyrName Then
    '            Return hndle
    '        End If
    '    Next
    '    Return Nothing
    'End Function

    Public Shared Function GetMWLayerWithLayerName _
         (map As MapWindow.Interfaces.IMapWin, lyrName As String) As MapWindow.Interfaces.Layer
        For Each lyr As Layer In map.Layers
            If Trim(lyr.Name) = lyrName Then
                Return lyr
            End If
        Next
        Return Nothing
    End Function

    Public Shared Function GetMWShapeFileWithLayerName _
     (map As MapWindow.Interfaces.IMapWin, lyrName As String) As MapWinGIS.Shapefile
        Dim sf As New MapWinGIS.Shapefile
        For Each lyr As Layer In map.Layers
            If Trim(lyr.Name) = lyrName Then
                sf.Open(lyr.FileName)
                Return sf
            End If
        Next
        Return Nothing
    End Function

    Public Shared Function GetMWGridWithLayerName _
 (map As MapWindow.Interfaces.IMapWin, lyrName As String) As MapWinGIS.Grid
        Dim grid As New MapWinGIS.Grid
        For Each lyr As Layer In map.Layers
            If Trim(lyr.Name) = lyrName Then
                grid.Open(lyr.FileName, GridDataType.UnknownDataType, True, GridFileType.UseExtension)
                Return grid
            End If
        Next
        Return Nothing
    End Function

    Public Shared Function GetMapXFromGrid(colX As Integer, baseGrid As MapWinGIS.Grid) As Double
        Dim x, y As Double
        baseGrid.CellToProj(colX, 0, x, y)
        Return x
    End Function

    Public Shared Function GetMapYFromGrid(rowY As Integer, baseGrid As MapWinGIS.Grid) As Double
        Dim x, y As Double
        baseGrid.CellToProj(0, rowY, x, y)
        Return y
    End Function

    Public Shared Function GetDimXFromGrid(x As Double, y As Double, baseGrid As MapWinGIS.Grid) As Integer
        Dim cx, ry As Integer
        baseGrid.ProjToCell(x, y, cx, ry)
        Return cx
    End Function

    Public Shared Function GetDimYFromGrid(x As Double, y As Double, baseGrid As MapWinGIS.Grid) As Integer
        Dim cx, ry As Integer
        baseGrid.ProjToCell(x, y, cx, ry)
        Return ry
    End Function

    Public Shared Function GetCellSizeFromGrid(baseGrid As MapWinGIS.Grid) As Single
        Dim dx As Double = baseGrid.Header.dX
        Dim dy As Double = baseGrid.Header.dY
        Return CSng((dx + dy) / 2)
    End Function

    Public Shared Function ChangeSymbolSize(inMap As AxMapWinGIS.AxMap, _
                                         inShpFile1 As MapWinGIS.Shapefile, inShpFile2 As MapWinGIS.Shapefile) As Boolean
        Dim zoomLevel As Single = cMap.GetZoomLevel(inMap)

        For i As Integer = 0 To inShpFile1.Categories.Count - 1
            Dim ct As ShapefileCategory = inShpFile1.Categories.Item(i)
            ct.DrawingOptions.PictureScaleX = zoomLevel
            ct.DrawingOptions.PictureScaleY = zoomLevel
        Next i

        For i As Integer = 0 To inShpFile2.Categories.Count - 1
            Dim ct As ShapefileCategory = inShpFile2.Categories.Item(i)
            ct.DrawingOptions.PictureScaleX = zoomLevel
            ct.DrawingOptions.PictureScaleY = zoomLevel
        Next i

        'Dim maxIT As Integer = inShpFile1.NumShapes
        'If inShpFile2.NumShapes > inShpFile1.NumShapes Then maxIT = inShpFile2.NumShapes
        'For sn As Integer = 0 To maxIT - 1
        '    If sn < inShpFile1.NumShapes Then
        '        inShpFile1.ShapeCategory3(sn).DrawingOptions.PictureScaleX = zoomLevel
        '        inShpFile1.ShapeCategory3(sn).DrawingOptions.PictureScaleY = zoomLevel
        '    End If
        '    If sn < inShpFile2.NumShapes Then
        '        inShpFile2.ShapeCategory3(sn).DrawingOptions.PictureScaleX = zoomLevel
        '        inShpFile2.ShapeCategory3(sn).DrawingOptions.PictureScaleY = zoomLevel
        '    End If
        'Next
        '이거 작동 안됨
        'inShpFile1.DefaultDrawingOptions.PictureScaleX = zoomLevel
        'inShpFile1.DefaultDrawingOptions.PictureScaleY = zoomLevel
        'inShpFile2.DefaultDrawingOptions.PictureScaleX = zoomLevel
        'inShpFile2.DefaultDrawingOptions.PictureScaleY = zoomLevel
    End Function


    Public Shared Function GetZoomLevel(inMap As AxMapWinGIS.AxMap) As Single
        Dim mapExt As New MapWinGIS.Extents
        Dim xMin As Double
        Dim yMin As Double
        Dim zMin As Double
        Dim xMax As Double
        Dim yMax As Double
        Dim zMax As Double
        mapExt = CType(inMap.Extents, MapWinGIS.Extents)
        mapExt.GetBounds(xMin, yMin, zMin, xMax, yMax, zMax)
        Dim cellSize As Single = cProject.Current.Watershed.mCellSize
        Dim cr As Single = CSng((xMax - xMin) / cellSize)
        Return CSng(1 / cr * 7.8)
    End Function

    Public Shared Sub SetFlowDirectionDisplay(inAxMap As AxMapWinGIS.AxMap, inWSGrid As MapWinGIS.Grid, _
                                           inShpPointFdirCH As MapWinGIS.Shapefile, inShpPointFdirOF As MapWinGIS.Shapefile, _
                                                inFpnFDBlue As String, inFpnFDGreen As String)
        Try

            If IO.File.Exists(inFpnFDBlue) = False AndAlso IO.File.Exists(inFpnFDGreen) = False Then Exit Sub

            Dim field As New Field
            Dim fidxFDA As Integer = 0
            With field
                .Name = "FDAngle"
                .Type = FieldType.INTEGER_FIELD
                .Width = 3
            End With
            inShpPointFdirOF.EditInsertField(field, fidxFDA)
            inShpPointFdirCH.EditInsertField(field, fidxFDA)

            Dim imgFDBlue As New MapWinGIS.Image
            Dim imgFDGreen As New MapWinGIS.Image
            imgFDBlue.Open(inFpnFDBlue, MapWinGIS.ImageType.USE_FILE_EXTENSION, True)
            imgFDGreen.Open(inFpnFDGreen, MapWinGIS.ImageType.USE_FILE_EXTENSION, True)
            imgFDBlue.UpsamplingMode = tkInterpolationMode.imNone
            imgFDGreen.UpsamplingMode = tkInterpolationMode.imNone

            'Dim zLevel As Single = cMap.GetZoomLevel(inAxMap)
            'Dim ct As MapWinGIS.ShapefileCategory
            
            'For rowY As Integer = 0 To cProject.Current.Watershed.mRowCountLayer - 1
            '    For colX As Integer = 0 To cProject.Current.Watershed.mColCountLayer - 1
            inShpPointFdirOF.StartEditingShapes()
            inShpPointFdirOF.StartEditingTable()
            inShpPointFdirCH.StartEditingShapes()
            inShpPointFdirCH.StartEditingTable()
            For ncv As Integer = 0 To cProject.Current.CVCount - 1
                Dim cv As cCVAttribute = cProject.Current.CV(ncv)
                'Dim wsid As Integer = CInt(inWSGrid.Value(colX, rowY)) '이 번호는 좌상단 부터 부여'cur.GetValue(intX, intY))
                'If wsid > 0 Then '유역 내부
                Dim newPoint As New MapWinGIS.Point
                newPoint.x = cv.MapX 'cProject.Current.WSCell(colX, rowY).MapX
                newPoint.y = cv.MapY 'cProject.Current.WSCell(colX, rowY).MapY 'cMap.GetMapY(colX, rowY, mGridWS)
                Dim newShp As New MapWinGIS.Shape
                newShp.Create(MapWinGIS.ShpfileType.SHP_POINT)
                newShp.InsertPoint(newPoint, 0)
                Dim fdAngle As Integer = cComTools.GetFDAngleNumber(cv.FDir) 'cProject.Current.WSCell(colX, rowY).FDir)
                ' If cProject.Current.WSCell(colX, rowY).FlowType = CellFlowType.OverlandFlow Then
                If cv.FlowType = CellFlowType.OverlandFlow Then
                    inShpPointFdirOF.EditInsertShape(newShp, inShpPointFdirOF.NumShapes)
                    'cMap.UpdateShapeFileField(inShpPointFdirOF, inShpPointFdirOF.NumShapes - 1, fidxFDA, fdAngle, False)
                    inShpPointFdirOF.EditCellValue(fidxFDA, inShpPointFdirOF.NumShapes - 1, fdAngle)
                    'ct = inShpPointFdirOF.Categories.Add("OF")
                    'Dim idCT As Integer = inShpPointFdirOF.Categories.Count - 1
                    'With ct.DrawingOptions
                    '    .PointSize = 0
                    '    .PointRotation = fdAngle
                    '    .PointType = MapWinGIS.tkPointSymbolType.ptSymbolPicture
                    '    .Picture = imgFDGreen
                    '    .PictureScaleX = zLevel
                    '    .PictureScaleY = zLevel
                    'End With
                    'inShpPointFdirOF.ShapeCategory(idCT) = idCT
                Else
                    'newShp.InsertPoint(newPoint, inShpPointFdirCH.NumShapes)
                    inShpPointFdirCH.EditInsertShape(newShp, inShpPointFdirCH.NumShapes)
                    'cMap.UpdateShapeFileField(inShpPointFdirCH, inShpPointFdirCH.NumShapes - 1, fidxFDA, fdAngle, False)
                    inShpPointFdirCH.EditCellValue(fidxFDA, inShpPointFdirCH.NumShapes - 1, fdAngle)
                    'ct = inShpPointFdirCH.Categories.Add("CH")
                    'Dim idCT As Integer = inShpPointFdirCH.Categories.Count - 1
                    'With ct.DrawingOptions
                    '    .PointSize = 0
                    '    .PointRotation = fdAngle
                    '    .PointType = MapWinGIS.tkPointSymbolType.ptSymbolPicture
                    '    .Picture = imgFDBlue
                    '    .PictureScaleX = zLevel
                    '    .PictureScaleY = zLevel
                    'End With
                    'inShpPointFdirCH.ShapeCategory(idCT) = idCT
                End If
                'End If
            Next ncv
            inShpPointFdirOF.StopEditingShapes()
            inShpPointFdirOF.StopEditingTable()
            inShpPointFdirCH.StopEditingShapes()
            inShpPointFdirCH.StopEditingTable()
            '    Next
            'Next
            Dim zLevel As Single = cMap.GetZoomLevel(inAxMap)
            cMap.SetFDCategory(inShpPointFdirOF, inShpPointFdirCH, field.Name, imgFDGreen, imgFDBlue, zLevel)
        Catch ex As Exception
            MsgBox(ex.ToString)
        End Try
    End Sub


    Public Shared Sub SetFDCategory(inShpPointFdirOF As MapWinGIS.Shapefile, _
                                    inShpPointFdirCH As MapWinGIS.Shapefile, _
                                    fieldName As String, _
                                    imgFDof As MapWinGIS.Image, _
                                    imgFDch As MapWinGIS.Image, _
                                    zoomLevel As Single)

        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "315", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "270", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "225", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "180", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "135", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "90", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "45", imgFDof, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirOF, fieldName, "0", imgFDof, zoomLevel)

        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "315", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "270", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "225", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "180", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "135", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "90", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "45", imgFDch, zoomLevel)
        cMap.SetCategoryWithShapeFileField(inShpPointFdirCH, fieldName, "0", imgFDch, zoomLevel)
    End Sub

    Public Shared Sub SetCategoryWithShapeFileField(shp As Shapefile, _
                                                    fieldName As String, value As String, _
                                                    img As MapWinGIS.Image, _
                                                    zLevel As Single)
        Dim ct As MapWinGIS.ShapefileCategory = shp.Categories.Add(value)
        ct.Expression = "[" + fieldName + "] = " + value
        With ct.DrawingOptions
            .PointSize = 0
            .PointRotation = CDbl(value)
            .PointType = tkPointSymbolType.ptSymbolPicture
            .Picture = img
            .PictureScaleX = zLevel
            .PictureScaleY = zLevel
        End With
    End Sub


    Public Shared Function DrawGridLines(inMap As AxMapWinGIS.AxMap, inWSGrid As MapWinGIS.Grid, _
                                       inShpGridLine As MapWinGIS.Shapefile) As Integer
        Dim maxCol As Integer = cProject.Current.Watershed.mColCountLayer
        Dim maxRow As Integer = cProject.Current.Watershed.mRowCountLayer
        Dim dblLineLeft As Double, dblLineTop As Double, dblLineRight As Double, dblLineBottom As Double
        Dim dblMapX As Double, dblMapY As Double
        dblLineLeft = cMap.GetMapXFromGrid(0, inWSGrid) - cMap.GetCellSizeFromGrid(inWSGrid) / 2
        dblLineRight = cMap.GetMapXFromGrid(maxCol - 1, inWSGrid) + cMap.GetCellSizeFromGrid(inWSGrid) / 2
        dblLineTop = cMap.GetMapYFromGrid(0, inWSGrid) + cMap.GetCellSizeFromGrid(inWSGrid) / 2
        dblLineBottom = cMap.GetMapYFromGrid(maxRow - 1, inWSGrid) - cMap.GetCellSizeFromGrid(inWSGrid) / 2
        For c As Integer = 0 To maxCol - 1
            dblMapX = cMap.GetMapXFromGrid(c, inWSGrid) - cMap.GetCellSizeFromGrid(inWSGrid) / 2
            If dblMapX <= dblLineRight And dblMapX >= dblLineLeft Then
                Dim newShp As New MapWinGIS.Shape
                newShp.Create(MapWinGIS.ShpfileType.SHP_POLYLINE)
                newShp.AddPoint(dblMapX, dblLineBottom)
                newShp.AddPoint(dblMapX, dblLineTop)
                inShpGridLine.EditInsertShape(newShp, inShpGridLine.NumShapes)
            End If
        Next
        '한줄 더 그린다.
        Dim newShpV As New MapWinGIS.Shape
        newShpV.Create(MapWinGIS.ShpfileType.SHP_POLYLINE)
        newShpV.AddPoint(dblLineRight, dblLineBottom)
        newShpV.AddPoint(dblLineRight, dblLineTop)
        inShpGridLine.EditInsertShape(newShpV, inShpGridLine.NumShapes)

        For r As Integer = 0 To maxRow - 1
            dblMapY = cMap.GetMapYFromGrid(r, inWSGrid) + cMap.GetCellSizeFromGrid(inWSGrid) / 2
            If dblMapY <= dblLineTop And dblMapY >= dblLineBottom Then
                Dim newShp As New MapWinGIS.Shape
                newShp.Create(MapWinGIS.ShpfileType.SHP_POLYLINE)
                newShp.AddPoint(dblLineLeft, dblMapY)
                newShp.AddPoint(dblLineRight, dblMapY)
                inShpGridLine.EditInsertShape(newShp, inShpGridLine.NumShapes)
            End If
        Next
        '한줄 더 그린다.
        Dim newShpH As New MapWinGIS.Shape
        newShpH.Create(MapWinGIS.ShpfileType.SHP_POLYLINE)
        newShpH.AddPoint(dblLineLeft, dblLineBottom)
        newShpH.AddPoint(dblLineRight, dblLineBottom)
        inShpGridLine.EditInsertShape(newShpH, inShpGridLine.NumShapes)

        'mShpLinesGrid.CollisionMode = MapWinGIS.tkCollisionMode.AllowCollisions
        Dim HandleShpGridLine As Integer
        HandleShpGridLine = inMap.AddLayer(inShpGridLine, True)
        Dim icolor As UInt32 = CUInt(RGB(0, 0, 0))
        inMap.set_ShapeLayerLineColor(HandleShpGridLine, icolor)

        Return HandleShpGridLine
        'For n As Integer = 0 To mShpGridLine.NumShapes - 1
        '    mGRMMap.set_ShapeLineColor(mHdlShpGridLine, n, icolor)
        'Next
    End Function


    Public Shared Sub AddCellPolygon(inBaseGrid As MapWinGIS.Grid, ByVal DimColX As Integer, ByVal DimRowY As Integer, inShp As MapWinGIS.Shapefile)
        Dim sPointX As Double = cMap.GetMapXFromGrid(DimColX, inBaseGrid) - cMap.GetCellSizeFromGrid(inBaseGrid) / 2
        Dim sPointY As Double = cMap.GetMapYFromGrid(DimRowY, inBaseGrid) + cMap.GetCellSizeFromGrid(inBaseGrid) / 2
        Dim ePointX As Double = cMap.GetMapXFromGrid(DimColX, inBaseGrid) + cMap.GetCellSizeFromGrid(inBaseGrid) / 2
        Dim ePointY As Double = cMap.GetMapYFromGrid(DimRowY, inBaseGrid) - cMap.GetCellSizeFromGrid(inBaseGrid) / 2
        Dim xpts() As Double = {sPointX, sPointX, ePointX, ePointX}
        Dim ypts() As Double = {sPointY, ePointY, ePointY, sPointY}
        Dim newShp As New MapWinGIS.Shape
        newShp.Create(MapWinGIS.ShpfileType.SHP_POLYGON)
        For n As Integer = 0 To xpts.Count - 1
            newShp.AddPoint(xpts(n), ypts(n))
        Next
        inShp.EditInsertShape(newShp, inShp.NumShapes)
        For n As Integer = 0 To inShp.NumFields - 1
            Dim f As MapWinGIS.Field
            f = inShp.Field(n)
            If f.Name = "ColX" Then
                inShp.EditCellValue(n, inShp.NumShapes - 1, DimColX)
            End If
            If f.Name = "RowY" Then
                inShp.EditCellValue(n, inShp.NumShapes - 1, DimRowY)
            End If
        Next
    End Sub

    Public Shared Sub AddFieldsColXandRowY(inSF As MapWinGIS.Shapefile)
        inSF.EditAddField("ColX", FieldType.INTEGER_FIELD, 0, 5)
        inSF.EditAddField("RowY", FieldType.INTEGER_FIELD, 0, 5)
    End Sub

    'Public Shared Sub AddAfiledToShapeFile(inSF As MapWinGIS.Shapefile, fieldName As String, fileType As MapWinGIS.FieldType)
    '    inSF.EditAddField(fieldName, FieldType.INTEGER_FIELD, 0, 5)
    'End Sub

    Public Shared Sub RemoveCellPoly(ByVal inSF As MapWinGIS.Shapefile, ByVal col As Integer, ByVal row As Integer)
        Dim fiColX As Integer = 0
        Dim fiRowY As Integer = 0
        For n As Integer = 0 To inSF.NumFields - 1
            Dim f As MapWinGIS.Field
            f = inSF.Field(n)
            If f.Name = "ColX" Then
                fiColX = n
            End If
            If f.Name = "RowY" Then
                fiRowY = n
            End If
        Next
        For n As Integer = 0 To inSF.NumShapes - 1
            If CInt(inSF.CellValue(fiColX, n)) = col AndAlso CInt(inSF.CellValue(fiRowY, n)) = row Then
                inSF.EditDeleteShape(n)
                Exit For
            End If
        Next
    End Sub

    Public Shared Sub RemoveAllRecordInShpFile(ByVal inSF As MapWinGIS.Shapefile)
        If inSF.NumShapes > 0 Then
            Dim count As Integer = 0
            Do Until inSF.NumShapes = 0
                For n As Integer = 0 To inSF.NumShapes - 1
                    inSF.EditDeleteShape(n)
                Next
                If count > 1000 Then
                    MsgBox(String.Format("Shp (in {0}) can not be removed in 1000 interation. ", inSF.Filename), _
                           MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                End If
            Loop
        End If
    End Sub


    Public Shared Function GetValueListIntegerInGridLayer(ByVal gridLayer As MapWinGIS.Grid) As List(Of Integer)
        Dim lst As New List(Of Integer)
        For c As Integer = 0 To gridLayer.Header.NumberCols - 1
            For r As Integer = 0 To gridLayer.Header.NumberRows - 1
                Dim v As Integer = CInt(gridLayer.Value(c, r))
                Dim bAdd As Boolean = True
                For Each lv As Integer In lst
                    If lv = v Then
                        bAdd = False
                        Exit For
                    End If
                Next
                If bAdd = True Then lst.Add(v)
            Next
        Next
        Return lst
    End Function

    Public Shared Function GetFieldNameList(inSF As MapWinGIS.Shapefile) As List(Of String)
        Dim lst As New List(Of String)
        For nf As Integer = 0 To inSF.NumFields - 1
            lst.Add(inSF.Field(nf).Name)
        Next
        Return lst
    End Function

    Public Shared Function GetFieldIndexByName(inSF As MapWinGIS.Shapefile, fieldName As String) As Integer
        For nf As Integer = 0 To inSF.NumFields - 1
            If fieldName = inSF.Field(nf).Name Then
                Return nf
            End If
        Next
        Return -1
    End Function

    Public Shared Function UpdateShapeFileField(shapefile As MapWinGIS.Shapefile, _
                                           fieldIndexToFind As Integer, _
                                           valueToFind As String, _
                                           fieldIndexToUpdate As Integer, _
                                           valueToUpdate As Single, _
                                           Optional bSave As Boolean = True) As Boolean
        Dim b As Boolean = False
        Dim c As Integer = 0
        Do While c < 1000
            For ns As Integer = 0 To shapefile.NumShapes - 1
                If shapefile.CellValue(fieldIndexToFind, ns).ToString = valueToFind.ToString Then
                    shapefile.StartEditingTable()
                    shapefile.EditCellValue(fieldIndexToUpdate, ns, valueToUpdate)
                    If shapefile.CellValue(fieldIndexToUpdate, ns) IsNot Nothing AndAlso _
                        shapefile.CellValue(fieldIndexToUpdate, ns).ToString = valueToUpdate.ToString Then
                        shapefile.StopEditingTable(True)
                        If bSave = True Then shapefile.Save()
                        Return True
                    End If
                    c += 1
                    Exit For
                End If
            Next
        Loop
        Return False
    End Function

    Public Shared Function UpdateShapeFileField(shapefile As MapWinGIS.Shapefile, _
                                       shpNumber As Integer, _
                                       fieldIndexToUpdate As Integer, _
                                       valueToUpdate As Single, Optional bSave As Boolean = True) As Boolean
        Dim b As Boolean = False
        Dim c As Integer = 0
        Do While c < 1000
            shapefile.StartEditingTable()
            shapefile.EditCellValue(fieldIndexToUpdate, shpNumber, valueToUpdate)
            If shapefile.CellValue(fieldIndexToUpdate, shpNumber) IsNot Nothing AndAlso _
               shapefile.CellValue(fieldIndexToUpdate, shpNumber).ToString = valueToUpdate.ToString Then
                shapefile.StopEditingTable(True)
                If bSave = True Then shapefile.Save()
                Return True
            End If
        Loop
        Return False
    End Function

    Public Shared Function GetNewFieldNameToAddToShapeFile(inshapeFile As MapWinGIS.Shapefile, _
                                           iniFieldName As String) As String
        Dim lstFName As List(Of String) = cMap.GetFieldNameList(inshapeFile)
        Dim b As Boolean = True
        Dim n As Integer = 0
        Dim rfFieldName As String = iniFieldName
        Do Until b = False
            n += 1
            b = False
            If lstFName.Contains(rfFieldName) Then
                rfFieldName = iniFieldName + n.ToString
                b = True
            End If
            If n > 1000 Then
                MsgBox("Adding template rainfall field failed...", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                Return "-1"
            End If
        Loop
        Return rfFieldName
    End Function

    Public Shared Function RemoveRFvalueField(inshapeFile As MapWinGIS.Shapefile, _
                                       iniFieldName As String) As String
        Dim lstFName As List(Of String) = cMap.GetFieldNameList(inshapeFile)
        Dim b As Boolean
        Dim n As Integer = 0
        Dim rfFieldName As String = iniFieldName
        Do Until b = False
            n = +1
            b = False
            If lstFName.Contains(rfFieldName) Then
                rfFieldName = rfFieldName + n.ToString
                b = True
            End If
            If n > 1000 Then
                MsgBox("Adding template rainfall field failed...", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                Return "-1"
            End If
        Loop
        Return rfFieldName
    End Function

End Class
