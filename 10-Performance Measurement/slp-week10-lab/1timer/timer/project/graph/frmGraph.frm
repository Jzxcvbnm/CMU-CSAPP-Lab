VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.Form frmGraph 
   Caption         =   "Timestamp Graph"
   ClientHeight    =   7500
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   12240
   LinkTopic       =   "Form1"
   ScaleHeight     =   7500
   ScaleWidth      =   12240
   StartUpPosition =   2  'CenterScreen
   Begin VB.PictureBox picNum 
      BorderStyle     =   0  'None
      Height          =   1095
      Index           =   1
      Left            =   9360
      ScaleHeight     =   1095
      ScaleWidth      =   375
      TabIndex        =   7
      Top             =   120
      Width           =   375
   End
   Begin VB.PictureBox picNum 
      BorderStyle     =   0  'None
      Height          =   1095
      Index           =   0
      Left            =   120
      ScaleHeight     =   1095
      ScaleWidth      =   375
      TabIndex        =   6
      Top             =   120
      Width           =   375
   End
   Begin MSComDlg.CommonDialog cd 
      Left            =   8880
      Top             =   7080
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CheckBox chkToggleOverlay 
      Caption         =   "Toggle Overlay"
      Height          =   255
      Left            =   5520
      TabIndex        =   5
      Top             =   7200
      Value           =   2  'Grayed
      Width           =   1455
   End
   Begin VB.CheckBox chkSingleLine 
      Caption         =   "Single Line"
      Height          =   255
      Left            =   4080
      TabIndex        =   4
      Top             =   7200
      Width           =   1335
   End
   Begin VB.CommandButton cmdOverlay 
      Caption         =   "Overla&y"
      Height          =   375
      Left            =   1920
      TabIndex        =   3
      Top             =   7080
      Width           =   1335
   End
   Begin VB.CommandButton cmdGraph 
      Caption         =   "&Select Data File"
      Height          =   375
      Left            =   480
      TabIndex        =   2
      Top             =   7080
      Width           =   1335
   End
   Begin VB.CommandButton cmdQuit 
      Caption         =   "Quit"
      Height          =   375
      Left            =   7440
      TabIndex        =   1
      Top             =   7080
      Width           =   1335
   End
   Begin VB.PictureBox pic 
      AutoRedraw      =   -1  'True
      BackColor       =   &H00C0C0C0&
      Height          =   1095
      Left            =   480
      ScaleHeight     =   1035
      ScaleWidth      =   8835
      TabIndex        =   0
      Top             =   120
      Width           =   8895
   End
End
Attribute VB_Name = "frmGraph"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

'  Graph is a graphing/charting program for exercise
'  6 of (minimially) v1.2 of SSD6.  Purpose is to graphically
'  depict concurrency, based on data from the timestamp.exe
'  program (of exercise 6).
'
'  For suggestions, bug fixes, complaints,
'  etc., contact Darren Siegel at d.siegel@icarnegie.com.

Option Explicit

Private Const BORDER As Integer = 120
Private mData As clsData

Private mfOverlayed As Boolean
Private mdInterval As Double
Private mdPt As Double

Private mfWaitingForClick As Boolean
Private mfSuppressOverlayForcedRedraw As Boolean
Private mlThreadColors(10) As Long

Private Sub chkSingleLine_Click()
    If Not mData Is Nothing Then DrawGraph
End Sub

Private Sub chkToggleOverlay_Click()
    If Not mfSuppressOverlayForcedRedraw Then DrawGraph
End Sub

Private Sub cmdGraph_Click()
    
    On Error GoTo ErrStart
    
    Static sLastPath As String
    If sLastPath = vbNullString Then
        cd.InitDir = App.Path
    Else
        cd.InitDir = sLastPath
    End If
    cd.Filter = "*.out|*.out"
    cd.DialogTitle = "Select timestamp.exe generated .out file"
    cd.CancelError = True
    
    On Error Resume Next
    cd.ShowOpen
    
    If Err.Number = cdlCancel Then
        Exit Sub
    ElseIf Err.Number = 0 Then
    
        If ExistFile(cd.FileName) Then
                
            sLastPath = GetDirFromFile(cd.FileName)
            Set mData = New clsData
            mfWaitingForClick = False
            mfOverlayed = False
            
            mfSuppressOverlayForcedRedraw = True
            chkToggleOverlay.Value = vbGrayed
            mfSuppressOverlayForcedRedraw = False
            
            mData.ReadData (cd.FileName)
            
            DrawGraph
        
        Else
        
            MsgBox "Error: Could not find file: " & cd.FileName
        End If
            
    Else
        On Error GoTo ErrStart
        Err.Raise Err.Number
    End If
    
    Exit Sub
ErrStart:
    MsgBox "Error in cmdGraph.click(): " & vbCrLf & Err.Number & " " & Err.Description
    
End Sub

Private Function ExistFile(sFileName As String) As Boolean
    
    On Error Resume Next
    Call FileLen(sFileName)
    ExistFile = (Err.Number = 0)

End Function

Private Function GetDirFromFile(sFileName As String) As String

    GetDirFromFile = Left$(sFileName, InStrRev(sFileName, "\") - 1)

End Function

Private Sub DrawGraph()
    
    On Error GoTo ErrStart
    
    Dim f As Double
    f = mData.ScaleFactor
    
    pic.Cls
    picNum(0).Cls
    picNum(1).Cls
    
    pic.Scale (mData.Min * f, 0)-(f * mData.Max, mData.ThreadCount * 10)
    picNum(0).Scale (0, 0)-(10, mData.ThreadCount * 10)
    picNum(1).Scale (0, 0)-(10, mData.ThreadCount * 10)
    
    Dim l As Long
    For l = 0 To mData.ThreadCount - 1
    
        Dim thread As clsThread
        Set thread = mData.thread(l)
        
        If chkSingleLine.Value = vbUnchecked Then
            DrawThreadNumber l
        End If
                        
        Dim lTime As Long
        For lTime = 0 To thread.TimeCount - 1
            If chkSingleLine.Value = vbChecked Then
                pic.Line (thread.StartTime(lTime) * f, 0)-(thread.EndTime(lTime) * f, 5), mlThreadColors(l), BF
            Else
                pic.Line (thread.StartTime(lTime) * f, l * 10)-(thread.EndTime(lTime) * f, (l * 10) + 5), mlThreadColors(l), BF
            End If
        Next
            
    Next
       
    If mfOverlayed And chkToggleOverlay.Value = vbChecked Then DrawOverlay
    
    For l = 0 To mData.ThreadCount - 1
    
        Set thread = mData.thread(l)
        For lTime = 0 To thread.TimeCount - 1
            If chkSingleLine.Value = vbChecked Then
                pic.Line (thread.EndTime(lTime) * f, 5)-(thread.EndTime(lTime) * f, 10), &HFF&
            Else
                pic.Line (thread.EndTime(lTime) * f, (l * 10) + 5)-(thread.EndTime(lTime) * f, (l * 10) + 10), &HFF&
            End If
        Next
            
    Next
    
    Exit Sub
ErrStart:
    MsgBox "Error in DrawGraph: " & vbCrLf & Err.Number & " " & Err.Description
    
End Sub

Private Sub cmdOverlay_Click()

    MsgBox "Click on the interrupt that you believe is a timer interrupt."
    mfWaitingForClick = True

End Sub

Private Sub cmdQuit_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    InitThreadColors
    SetNumFont
End Sub

Private Sub SetNumFont()

    Dim i As Integer
    For i = 0 To 1
        picNum(i).Font.Name = "Tahoma"
        picNum(i).Font.Size = 12
        picNum(i).Font.Bold = True
    Next

End Sub

Private Sub InitThreadColors()
 
    mlThreadColors(0) = &H8000&
    mlThreadColors(1) = &H800000
    mlThreadColors(2) = &H80&
    mlThreadColors(3) = &H808000
    mlThreadColors(4) = &H8080&
    mlThreadColors(5) = &H800080
    mlThreadColors(6) = &HFF0000
    mlThreadColors(7) = &HFF00&
    mlThreadColors(8) = &HFF&
    mlThreadColors(9) = &HFFFFFF


End Sub
Private Sub Form_Resize()

    ' Since this is a resize event, we can safely
    ' ignore errors due to really small sizes.
    On Error Resume Next

    ' Resize the picture box
    pic.Left = BORDER + picNum(0).Width
    pic.Top = BORDER
    pic.Width = Abs(Me.ScaleWidth) - (2 * BORDER) - (picNum(0).Width * 2)
    pic.Height = Abs(Me.ScaleHeight) - (3 * BORDER) - cmdQuit.Height
    
    ' Resize the number picture box
    picNum(0).Left = BORDER
    picNum(0).Top = BORDER
    picNum(0).Height = pic.Height
    picNum(1).Left = BORDER + picNum(0).Width + pic.Width
    picNum(1).Top = BORDER
    picNum(1).Height = pic.Height
    
    ' And place the command buttons and other controls
    cmdQuit.Top = Abs(Me.ScaleHeight) - cmdQuit.Height - (BORDER)
    cmdGraph.Top = Abs(Me.ScaleHeight) - cmdGraph.Height - (BORDER)
    cmdOverlay.Top = Abs(Me.ScaleHeight) - cmdOverlay.Height - (BORDER)
    cmdQuit.Left = Abs(Me.ScaleWidth) - cmdQuit.Width - BORDER - picNum(0).Width
    chkSingleLine.Top = cmdOverlay.Top
    chkToggleOverlay.Top = cmdOverlay.Top
    
    If Not mData Is Nothing Then DrawGraph
    
End Sub

Private Function f(d As Double) As String

    f = Format$(d, "###,###,###,###,###")

End Function

Private Sub pic_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)

    On Error GoTo ErrStart
    
    If Not mData Is Nothing Then
        Dim pt As Double
        pt = X / mData.ScaleFactor
    
        Dim lThread As Long
        Dim thread As clsThread
        Dim lTime As Long
        
        If chkSingleLine.Value = vbChecked Then
            mData.FindClosestInt pt, lThread, lTime
            Set thread = mData.thread(lThread)
        Else
            lThread = Int(Y / 10)
            If lThread + 1 > mData.ThreadCount Then Exit Sub
            Set thread = mData.thread(lThread)
            thread.FindClosestInt pt, lTime
            
        End If
    
        Dim dMin As Double
        dMin = Abs(thread.EndTime(lTime) - pt)
       
        If ((mData.Max - mData.Min) * 0.01 > dMin) Then
            Dim et As Double, stNext As Double
            et = thread.EndTime(lTime)
            
            Dim adjusted As Double
            If mData.Min < 0 Then
                If et < 0 Then
                    adjusted = Abs(mData.Min) - Abs(et)
                Else
                    adjusted = Abs(mData.Min) + et
                End If
            Else
                adjusted = et
            End If
            
            If lTime < thread.TimeCount - 1 Then
                stNext = thread.StartTime(lTime + 1)
                pic.ToolTipText = "Interrupt occurred at " & f(adjusted) & " and lasted " & f(stNext - et)
            Else
                pic.ToolTipText = "Interrupt occurred at " & f(adjusted)
            End If
            
        Else
            pic.ToolTipText = vbNullString
        End If
    
    End If
    
    Exit Sub
ErrStart:
    MsgBox "Error in pic.MouseMove: " & vbCrLf & Err.Number & " " & Err.Description

End Sub

Private Sub pic_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)

    On Error GoTo ErrStart

    If mfWaitingForClick Then
    
        mfWaitingForClick = False
        
        Dim pt As Double
        pt = X / mData.ScaleFactor
        
        Dim thread As clsThread
        Dim intPt As Double, lTime As Long
        Dim lThread As Long
        
        If chkSingleLine.Value = vbChecked Then
            mData.FindClosestInt pt, lThread, lTime
            Set thread = mData.thread(lThread)
        Else
            
            lThread = Int(Y / 10)
            If lThread + 1 > mData.ThreadCount Then Exit Sub
           
            Set thread = mData.thread(lThread)
            thread.FindClosestInt pt, lTime
            
        End If
        
        intPt = thread.EndTime(lTime)
           
        Do
            Dim dInterval As Double
            Dim sInput As String
            
            sInput = (InputBox("Enter what you believe is the timer interval (in cycles).  Enter only a positive value."))
            If (sInput = vbNullString) Then Exit Do
            
            If IsNumeric(sInput) Then
                
                dInterval = CDbl(sInput)
                If dInterval > 0 Then
        
                    mdInterval = dInterval * mData.ScaleFactor
                    mdPt = intPt * mData.ScaleFactor
                    mfOverlayed = True
                    chkToggleOverlay.Value = vbChecked
            
                    DrawGraph
                    Exit Do
                
                End If
                
            End If
            
        Loop
        
    End If
    
    Exit Sub
ErrStart:
    MsgBox "Error in pic.MouseUp: " & vbCrLf & Err.Number & " " & Err.Description
    
End Sub

Private Sub DrawOverlay()

    On Error GoTo ErrStart

    pic.DrawStyle = vbDot

    Dim f As Double
    f = mData.ScaleFactor
    Dim dCurrent As Double
    dCurrent = mdPt
    Do While (dCurrent > mData.Min * f)
       
        pic.Line (dCurrent, 0)-(dCurrent, 100), &H808080
        dCurrent = dCurrent - mdInterval
    Loop
    
    dCurrent = mdPt + mdInterval
    Do While (dCurrent < mData.Max * f)
        pic.Line (dCurrent, 0)-(dCurrent, 100), &H808080
        dCurrent = dCurrent + mdInterval
    Loop
    
    pic.DrawStyle = vbSolid
    
    Exit Sub
ErrStart:
    MsgBox "Error in DrawOverlay: " & vbCrLf & Err.Number & " " & Err.Description
    
End Sub

Private Sub DrawThreadNumber(lThreadNum As Long)

    ' Draw thread number, centered horizontally (and vertically)
    Dim i As Integer
    For i = 0 To 1
    
        picNum(i).CurrentX = 5 - (Abs(picNum(i).TextWidth(CStr(lThreadNum))) / 2)
        Dim lY As Long
        lY = (lThreadNum * 10) + 3
        picNum(i).CurrentY = lY - (Abs(picNum(i).TextHeight(CStr(lThreadNum))) / 2)
        picNum(i).Print CStr(lThreadNum)
        
    Next

End Sub
