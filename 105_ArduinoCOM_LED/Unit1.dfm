object Form1: TForm1
  Left = 399
  Top = 148
  Width = 730
  Height = 629
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
  object Label2: TLabel
    Left = 64
    Top = 160
    Width = 577
    Height = 41
    AutoSize = False
    Caption = 'Label2'
    Color = clWhite
    ParentColor = False
  end
  object GroupBox1: TGroupBox
    Left = 64
    Top = 72
    Width = 393
    Height = 73
    Caption = 'ComPort'
    TabOrder = 0
    object ComLed1: TComLed
      Left = 216
      Top = 32
      Width = 25
      Height = 25
      ComPort = ComPort1
      LedSignal = lsConn
      Kind = lkRedLight
    end
    object Button1: TButton
      Left = 24
      Top = 32
      Width = 75
      Height = 25
      Caption = 'Connect'
      TabOrder = 0
      OnClick = Button1Click
    end
    object Button2: TButton
      Left = 120
      Top = 32
      Width = 75
      Height = 25
      Caption = 'Disconnect'
      TabOrder = 1
      OnClick = Button2Click
    end
    object Button5: TButton
      Left = 296
      Top = 32
      Width = 75
      Height = 25
      Caption = 'Quit'
      TabOrder = 2
      OnClick = Button5Click
    end
  end
  object GroupBox2: TGroupBox
    Left = 64
    Top = 400
    Width = 577
    Height = 65
    Caption = 'LED_BUILTIN'
    TabOrder = 1
    object Label1: TLabel
      Left = 480
      Top = 24
      Width = 40
      Height = 25
      Alignment = taCenter
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Shape1: TShape
      Left = 536
      Top = 24
      Width = 25
      Height = 25
      Shape = stCircle
    end
    object Button3: TButton
      Left = 120
      Top = 24
      Width = 49
      Height = 25
      Caption = 'Off'
      TabOrder = 0
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 184
      Top = 24
      Width = 49
      Height = 25
      Caption = 'On'
      TabOrder = 1
      OnClick = Button4Click
    end
    object ScrollBar1: TScrollBar
      Left = 248
      Top = 24
      Width = 201
      Height = 25
      LargeChange = 10
      PageSize = 0
      TabOrder = 2
      OnChange = ScrollBar1Change
    end
    object Edit1: TEdit
      Left = 8
      Top = 24
      Width = 89
      Height = 25
      TabOrder = 3
      Text = 'Edit1'
    end
  end
  object ComPort1: TComPort
    BaudRate = br115200
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
    OnRxChar = ComPort1RxChar
    Left = 472
    Top = 96
  end
end
