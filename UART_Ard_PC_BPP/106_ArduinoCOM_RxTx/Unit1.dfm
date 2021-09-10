object Form1: TForm1
  Left = 232
  Top = 119
  Width = 730
  Height = 918
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
    Left = 56
    Top = 128
    Width = 577
    Height = 60
    AutoSize = False
    Caption = 'Label2'
    Color = clWhite
    ParentColor = False
  end
  object GroupBox1: TGroupBox
    Left = 56
    Top = 40
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
    Left = 56
    Top = 560
    Width = 580
    Height = 55
    Caption = 'DPINx'
    TabOrder = 1
    object Label1: TLabel
      Left = 480
      Top = 16
      Width = 40
      Height = 20
      Alignment = taCenter
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Shape1: TShape
      Left = 536
      Top = 16
      Width = 25
      Height = 20
      Shape = stCircle
    end
    object Button3: TButton
      Left = 120
      Top = 16
      Width = 50
      Height = 20
      Caption = 'Off'
      TabOrder = 0
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 184
      Top = 16
      Width = 50
      Height = 20
      Caption = 'On'
      TabOrder = 1
      OnClick = Button4Click
    end
    object ScrollBar1: TScrollBar
      Left = 248
      Top = 16
      Width = 201
      Height = 20
      LargeChange = 10
      PageSize = 0
      TabOrder = 2
      OnChange = ScrollBar1Change
    end
    object Edit1: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 21
      TabOrder = 3
      Text = 'LEDBI'
      OnChange = Edit1Change
    end
  end
  object GroupBox3: TGroupBox
    Left = 56
    Top = 200
    Width = 180
    Height = 60
    Caption = 'var i0'
    TabOrder = 2
    object Label3: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit3: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT0'
      OnChange = Edit3Change
    end
  end
  object GroupBox4: TGroupBox
    Left = 256
    Top = 200
    Width = 180
    Height = 60
    Caption = 'var i1'
    TabOrder = 3
    object Label4: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit4: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT1'
      OnChange = Edit4Change
    end
  end
  object GroupBox5: TGroupBox
    Left = 456
    Top = 200
    Width = 180
    Height = 60
    Caption = 'var i2'
    TabOrder = 4
    object Label5: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit5: TEdit
      Left = 0
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT2'
      OnChange = Edit5Change
    end
  end
  object Edit2: TEdit
    Left = 544
    Top = 56
    Width = 89
    Height = 21
    TabOrder = 5
    Text = 'Test 105'
  end
  object GroupBox6: TGroupBox
    Left = 56
    Top = 264
    Width = 180
    Height = 60
    Caption = 'var i3'
    TabOrder = 6
    object Label6: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit6: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT3'
      OnChange = Edit6Change
    end
  end
  object GroupBox7: TGroupBox
    Left = 256
    Top = 264
    Width = 180
    Height = 60
    Caption = 'var i4'
    TabOrder = 7
    object Label7: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit7: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT4'
      OnChange = Edit7Change
    end
  end
  object GroupBox8: TGroupBox
    Left = 456
    Top = 264
    Width = 180
    Height = 60
    Caption = 'var i5'
    TabOrder = 8
    object Label8: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit8: TEdit
      Left = 0
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT5'
      OnChange = Edit8Change
    end
  end
  object GroupBox9: TGroupBox
    Left = 56
    Top = 328
    Width = 180
    Height = 60
    Caption = 'var i6'
    TabOrder = 9
    object Label9: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit9: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT6'
      OnChange = Edit9Change
    end
  end
  object GroupBox10: TGroupBox
    Left = 256
    Top = 328
    Width = 180
    Height = 60
    Caption = 'var i7'
    TabOrder = 10
    object Label10: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit10: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT7'
      OnChange = Edit10Change
    end
  end
  object GroupBox11: TGroupBox
    Left = 456
    Top = 328
    Width = 180
    Height = 60
    Caption = 'var i8'
    TabOrder = 11
    object Label11: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit11: TEdit
      Left = 0
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'INT8'
      OnChange = Edit11Change
    end
  end
  object GroupBox12: TGroupBox
    Left = 56
    Top = 416
    Width = 180
    Height = 60
    Caption = 'var d0'
    TabOrder = 12
    object Label12: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit12: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'DBL0'
      OnChange = Edit12Change
    end
  end
  object GroupBox13: TGroupBox
    Left = 256
    Top = 416
    Width = 180
    Height = 60
    Caption = 'var d1'
    TabOrder = 13
    object Label13: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit13: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'DBL1'
      OnChange = Edit13Change
    end
  end
  object GroupBox14: TGroupBox
    Left = 456
    Top = 416
    Width = 180
    Height = 60
    Caption = 'var d2'
    TabOrder = 14
    object Label14: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit14: TEdit
      Left = 0
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'DBL2'
      OnChange = Edit14Change
    end
  end
  object GroupBox15: TGroupBox
    Left = 56
    Top = 480
    Width = 180
    Height = 60
    Caption = 'var d3'
    TabOrder = 15
    object Label15: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit15: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'DBL3'
      OnChange = Edit15Change
    end
  end
  object GroupBox16: TGroupBox
    Left = 256
    Top = 480
    Width = 180
    Height = 60
    Caption = 'var d4'
    TabOrder = 16
    object Label16: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit16: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'DBL4'
      OnChange = Edit16Change
    end
  end
  object GroupBox17: TGroupBox
    Left = 456
    Top = 480
    Width = 180
    Height = 60
    Caption = 'var d5'
    TabOrder = 17
    object Label17: TLabel
      Left = 80
      Top = 24
      Width = 90
      Height = 25
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Edit17: TEdit
      Left = 0
      Top = 24
      Width = 60
      Height = 21
      TabOrder = 0
      Text = 'DBL5'
      OnChange = Edit17Change
    end
  end
  object GroupBox18: TGroupBox
    Left = 56
    Top = 624
    Width = 580
    Height = 55
    Caption = 'DPINx'
    TabOrder = 18
    object Label18: TLabel
      Left = 480
      Top = 16
      Width = 40
      Height = 20
      Alignment = taCenter
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Shape2: TShape
      Left = 536
      Top = 16
      Width = 25
      Height = 20
      Shape = stCircle
    end
    object Button6: TButton
      Left = 120
      Top = 16
      Width = 50
      Height = 20
      Caption = 'Off'
      TabOrder = 0
      OnClick = Button6Click
    end
    object Button7: TButton
      Left = 184
      Top = 16
      Width = 50
      Height = 20
      Caption = 'On'
      TabOrder = 1
      OnClick = Button7Click
    end
    object ScrollBar2: TScrollBar
      Left = 248
      Top = 16
      Width = 201
      Height = 20
      LargeChange = 10
      PageSize = 0
      TabOrder = 2
      OnChange = ScrollBar2Change
    end
    object Edit18: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 21
      TabOrder = 3
      Text = 'DPIN10'
      OnChange = Edit18Change
    end
  end
  object GroupBox19: TGroupBox
    Left = 56
    Top = 688
    Width = 580
    Height = 55
    Caption = 'DPINx'
    TabOrder = 19
    object Label19: TLabel
      Left = 480
      Top = 16
      Width = 40
      Height = 20
      AutoSize = False
      Color = clWhite
      ParentColor = False
    end
    object Shape3: TShape
      Left = 536
      Top = 16
      Width = 25
      Height = 20
      Shape = stCircle
    end
    object Edit19: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 21
      TabOrder = 0
      Text = 'DPIN11'
      OnChange = Edit19Change
    end
    object Button8: TButton
      Left = 120
      Top = 16
      Width = 50
      Height = 20
      Caption = 'Off'
      TabOrder = 1
      OnClick = Button8Click
    end
    object Button9: TButton
      Left = 184
      Top = 16
      Width = 50
      Height = 20
      Caption = 'On'
      TabOrder = 2
      OnClick = Button9Click
    end
    object ScrollBar3: TScrollBar
      Left = 248
      Top = 16
      Width = 193
      Height = 20
      LargeChange = 10
      PageSize = 0
      TabOrder = 3
      OnChange = ScrollBar3Change
    end
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
    OnRxChar = ComPort1RxChar
    Left = 464
    Top = 64
  end
end
