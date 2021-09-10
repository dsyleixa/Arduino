object Form1: TForm1
  Left = 300
  Top = 169
  Width = 563
  Height = 249
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 16
    Top = 24
    Width = 209
    Height = 65
    Caption = 'Serial Port'
    TabOrder = 0
    object Button1: TButton
      Left = 16
      Top = 24
      Width = 75
      Height = 25
      Caption = 'Connect'
      Default = True
      TabOrder = 0
      OnClick = Button1Click
    end
    object Button2: TButton
      Left = 112
      Top = 24
      Width = 75
      Height = 25
      Caption = 'Disconnect'
      TabOrder = 1
      OnClick = Button2Click
    end
  end
  object GroupBox2: TGroupBox
    Left = 248
    Top = 24
    Width = 265
    Height = 105
    Caption = 'LED_BUILTIN'
    TabOrder = 1
    object Shape1: TShape
      Left = 216
      Top = 16
      Width = 33
      Height = 33
      Brush.Color = clBlack
      Shape = stCircle
    end
    object Label1: TLabel
      Left = 200
      Top = 64
      Width = 57
      Height = 33
      Align = alCustom
      Alignment = taCenter
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'Courier'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Button3: TButton
      Left = 16
      Top = 24
      Width = 75
      Height = 25
      Caption = 'Off'
      Default = True
      TabOrder = 0
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 104
      Top = 24
      Width = 75
      Height = 25
      Caption = 'On'
      Enabled = False
      TabOrder = 1
      OnClick = Button4Click
    end
    object ScrollBar1: TScrollBar
      Left = 16
      Top = 64
      Width = 161
      Height = 17
      PageSize = 0
      TabOrder = 2
      OnChange = ScrollBar1Change
    end
  end
  object Button5: TButton
    Left = 88
    Top = 96
    Width = 75
    Height = 25
    Caption = 'Quit'
    TabOrder = 2
    OnClick = Button5Click
  end
  object ComPort1: TComPort
    BaudRate = br115200
    Port = 'COM1'
    Parity.Bits = prNone
    StopBits = sbOneStopBit
    DataBits = dbEight
    Events = [evRxChar, evTxEmpty, evRxFlag, evRing, evBreak, evCTS, evDSR, evError, evRLSD, evRx80Full]
    FlowControl.OutCTSFlow = False
    FlowControl.OutDSRFlow = False
    FlowControl.ControlDTR = dtrDisable
    FlowControl.ControlRTS = rtsDisable
    FlowControl.XonXoffOut = False
    FlowControl.XonXoffIn = False
    StoredProps = [spBasic]
    TriggersOnRxChar = True
    Left = 32
    Top = 96
  end
end
