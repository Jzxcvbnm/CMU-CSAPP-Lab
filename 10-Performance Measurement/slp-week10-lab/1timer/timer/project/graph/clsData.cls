VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "clsData"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private mThreads() As clsThread
Private mlThreadCount As Long
Private mdMax As Double
Private mdMin As Double

Public Property Get thread(lIndex As Long) As clsThread
    Set thread = mThreads(lIndex)
End Property

Public Property Get Max() As Double
    Max = mdMax
End Property

Public Property Get Min() As Double
    Min = mdMin
End Property

Public Property Get ThreadCount() As Long

    If mlThreadCount > 10 Then
        ThreadCount = 10
    Else
        ThreadCount = mlThreadCount
    End If
    
End Property

Public Sub ReadData(sFileName As String)

    mdMin = -1
    Dim iFileNum As Integer
    iFileNum = FreeFile
    Open sFileName For Input As #iFileNum
    
    Dim lThread As Long
    Dim dStart As Double, dEnd As Double
    Dim thread As clsThread
        
    Do Until EOF(iFileNum)
    
        Input #iFileNum, lThread, dStart, dEnd
        
        If (lThread + 1 > mlThreadCount) Then
            mlThreadCount = lThread + 1
            ReDim Preserve mThreads(mlThreadCount - 1)
            Set thread = New clsThread
            thread.ThreadID = lThread
            Set mThreads(lThread) = thread
        End If
        
        If (mThreads(lThread) Is Nothing) Then
            Set thread = New clsThread
            thread.ThreadID = lThread
            Set mThreads(lThread) = thread
        End If
            
        Set thread = mThreads(lThread)
        
        
        If lThread = 0 Then
            Dim test As Integer
            test = test + 1
           ' If test = 7 Then Stop
        End If
        
        thread.AddData dStart, dEnd
        
       
        
        If dEnd > mdMax Then mdMax = dEnd
        If dStart < mdMin Or mdMin = -1 Then mdMin = dStart
        
    Loop
    
    Close #iFileNum

End Sub

Public Property Get ScaleFactor() As Double

    If Len(CStr(mdMax)) > 9 Then
    
        ScaleFactor = 1 / (10 ^ (Len(CStr(mdMax)) - 9))
    
    Else
        ScaleFactor = 1#
    End If

End Property

Public Sub FindClosestInt(cycles As Double, lThread As Long, lTime As Long)

    Dim thread As clsThread
    Set thread = mThreads(0)
    
    Dim lT As Long
    Dim dMin As Double, lMin As Long, lBestThread As Long
    dMin = Abs(thread.EndTime(0) - cycles)
    lMin = 0
    
    For lT = 0 To mlThreadCount - 1
        Set thread = mThreads(lT)
        Dim l As Long
        For l = 0 To thread.TimeCount - 1
            If Abs(thread.EndTime(l) - cycles) < dMin Then
                dMin = Abs(thread.EndTime(l) - cycles)
                lBestThread = lT
                lMin = l
            End If
        Next
    Next
    
    lThread = lBestThread
    lTime = lMin

End Sub

