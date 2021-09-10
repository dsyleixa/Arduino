object Form1: TForm1
  Left = 277
  Top = 146
  Width = 563
  Height = 291
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
    Left = 48
    Top = 40
    Width = 209
    Height = 73
    Caption = 'Serial Port'
    TabOrder = 0
    object Button1: TButton
      Left = 16
      Top = 24
      Width = 75
      Height = 25
      Caption = 'Connect'
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
    Left = 312
    Top = 40
    Width = 177
    Height = 177
    Caption = 'LED_BUILTIN'
    TabOrder = 1
    object Shape1: TShape
      Left = 56
      Top = 48
      Width = 65
      Height = 65
      Shape = stCircle
    end
    object Button3: TButton
      Left = 8
      Top = 144
      Width = 75
      Height = 25
      Caption = 'On'
      Enabled = False
      TabOrder = 0
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 96
      Top = 144
      Width = 75
      Height = 25
      Caption = 'Off'
      TabOrder = 1
      OnClick = Button4Click
    end
  end
  object Button5: TButton
    Left = 112
    Top = 136
    Width = 75
    Height = 25
    Caption = 'Quit'
    TabOrder = 2
    OnClick = Button5Click
  end
  object ComPort1: TComPort
    BaudRate = br9600
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
    Left = 136
    Top = 16
  end
end
