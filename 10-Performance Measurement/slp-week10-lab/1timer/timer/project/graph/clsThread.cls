VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "clsThread"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private miThreadID As Integer
Private mdStart() As Double
Private mdEnd() As Double
Private mlCount As Long

Public Property Get TimeCount() As Integer
    TimeCount = mlCount
End Property

Public Property Get ThreadID() As Integer
    ThreadID = miThreadID
End Property

Public Property Let ThreadID(i As Integer)
    miThreadID = i
End Property

Public Sub AddData(dStart As Double, dEnd As Double)

    ReDim Preserve mdStart(mlCount)
    ReDim Preserve mdEnd(mlCount)
    mdStart(mlCount) = dStart
    mdEnd(mlCount) = dEnd
    
    mlCount = mlCount + 1
        
End Sub

Public Property Get StartTime(lIndex As Long) As Double
    StartTime = mdStart(lIndex)
End Property

Public Property Get EndTime(lIndex As Long) As Double
    EndTime = mdEnd(lIndex)
End Property

Public Sub FindClosestInt(cycles As Double, lTime As Long)

    Dim dMin As Double, lMin As Long
    dMin = Abs(mdEnd(0) - cycles)
    lMin = 0
    Dim l As Long
    
    For l = 1 To mlCount - 1
        If Abs(mdEnd(l) - cycles) < dMin Then
            dMin = Abs(mdEnd(l) - cycles)
            lMin = l
        End If
    Next
    
    lTime = lMin
    
End Sub
