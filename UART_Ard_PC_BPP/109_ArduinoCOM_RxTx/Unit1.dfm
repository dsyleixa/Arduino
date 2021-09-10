object Form1: TForm1
  Left = 363
  Top = 164
  Width = 1310
  Height = 848
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Times New Roman'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 15
  object Label2: TLabel
    Left = 656
    Top = 80
    Width = 585
    Height = 57
    AutoSize = False
    Caption = 'Label2'
    Color = clBlack
    Font.Charset = ANSI_CHARSET
    Font.Color = clWhite
    Font.Height = -15
    Font.Name = 'Courier New'
    Font.Style = []
    ParentColor = False
    ParentFont = False
  end
  object Label26: TLabel
    Left = 432
    Top = 16
    Width = 417
    Height = 49
    Alignment = taCenter
    AutoSize = False
    Caption = 'Arduino Dashboard'
    Color = clBtnFace
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -32
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentColor = False
    ParentFont = False
  end
  object GroupBox10: TGroupBox
    Left = 640
    Top = 170
    Width = 601
    Height = 583
    Caption = 'OUTPUTS'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Times New Roman'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 21
    object GroupBox11: TGroupBox
      Left = 16
      Top = 224
      Width = 129
      Height = 193
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      object Label27: TLabel
        Left = 8
        Top = 128
        Width = 65
        Height = 25
        AutoSize = False
        Caption = ' '
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Shape4: TShape
        Left = 32
        Top = 160
        Width = 25
        Height = 20
        Shape = stCircle
      end
      object ScrollBar4: TScrollBar
        Left = 88
        Top = 24
        Width = 25
        Height = 153
        Kind = sbVertical
        LargeChange = 10
        Max = 0
        PageSize = 0
        TabOrder = 0
        OnChange = ScrollBar4Change
      end
      object Button11: TButton
        Left = 8
        Top = 96
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button11Click
      end
      object Button12: TButton
        Left = 8
        Top = 56
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = Button12Click
      end
      object Edit27: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnChange = Edit27Change
      end
    end
    object GroupBox29: TGroupBox
      Left = 160
      Top = 224
      Width = 129
      Height = 193
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      object Label28: TLabel
        Left = 8
        Top = 128
        Width = 65
        Height = 25
        AutoSize = False
        Caption = ' '
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Shape5: TShape
        Left = 32
        Top = 160
        Width = 25
        Height = 20
        Shape = stCircle
      end
      object ScrollBar5: TScrollBar
        Left = 88
        Top = 24
        Width = 25
        Height = 153
        Kind = sbVertical
        LargeChange = 10
        PageSize = 0
        TabOrder = 0
        OnChange = ScrollBar5Change
      end
      object Button13: TButton
        Left = 8
        Top = 88
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button13Click
      end
      object Button14: TButton
        Left = 8
        Top = 56
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = Button14Click
      end
      object Edit28: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnChange = Edit28Change
      end
    end
    object GroupBox30: TGroupBox
      Left = 304
      Top = 224
      Width = 129
      Height = 193
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      object Label29: TLabel
        Left = 8
        Top = 128
        Width = 65
        Height = 25
        AutoSize = False
        Caption = ' '
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Shape6: TShape
        Left = 32
        Top = 160
        Width = 25
        Height = 20
        Shape = stCircle
      end
      object ScrollBar6: TScrollBar
        Left = 88
        Top = 24
        Width = 25
        Height = 153
        Kind = sbVertical
        LargeChange = 10
        PageSize = 0
        TabOrder = 0
        OnChange = ScrollBar6Change
      end
      object Button15: TButton
        Left = 8
        Top = 88
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button15Click
      end
      object Button16: TButton
        Left = 8
        Top = 56
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = Button16Click
      end
      object Edit29: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnChange = Edit29Change
      end
    end
    object GroupBox31: TGroupBox
      Left = 448
      Top = 224
      Width = 129
      Height = 193
      Caption = 'LEDBI, OUT2, OUT3,..'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      object Label30: TLabel
        Left = 8
        Top = 128
        Width = 65
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Shape7: TShape
        Left = 32
        Top = 160
        Width = 20
        Height = 20
        Shape = stCircle
      end
      object Edit30: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'LEDBI'
        OnChange = Edit30Change
      end
      object Button17: TButton
        Left = 8
        Top = 88
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button17Click
      end
      object Button18: TButton
        Left = 8
        Top = 56
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = Button18Click
      end
      object ScrollBar7: TScrollBar
        Left = 88
        Top = 24
        Width = 25
        Height = 153
        Kind = sbVertical
        LargeChange = 10
        PageSize = 0
        TabOrder = 3
        OnChange = ScrollBar7Change
      end
    end
    object GroupBox32: TGroupBox
      Left = 16
      Top = 424
      Width = 129
      Height = 150
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      object Label31: TLabel
        Left = 88
        Top = 24
        Width = 30
        Height = 25
        Alignment = taCenter
        AutoSize = False
        Caption = 'O'
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'Courier New'
        Font.Style = [fsBold]
        ParentColor = False
        ParentFont = False
      end
      object Shape8: TShape
        Left = 88
        Top = 82
        Width = 30
        Height = 20
        Shape = stCircle
      end
      object Button19: TButton
        Left = 8
        Top = 64
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = Button19Click
      end
      object Button20: TButton
        Left = 8
        Top = 104
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button20Click
      end
      object Edit31: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        Text = 'OUT2'
        OnChange = Edit31Change
      end
    end
    object GroupBox33: TGroupBox
      Left = 160
      Top = 424
      Width = 129
      Height = 150
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 5
      object Label32: TLabel
        Left = 88
        Top = 24
        Width = 30
        Height = 25
        Alignment = taCenter
        AutoSize = False
        Caption = 'O'
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'Courier New'
        Font.Style = [fsBold]
        ParentColor = False
        ParentFont = False
      end
      object Shape9: TShape
        Left = 88
        Top = 82
        Width = 30
        Height = 20
        Shape = stCircle
      end
      object Button21: TButton
        Left = 8
        Top = 64
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = Button21Click
      end
      object Button22: TButton
        Left = 8
        Top = 104
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button22Click
      end
      object Edit32: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        Text = 'OUT3'
        OnChange = Edit32Change
      end
    end
    object GroupBox34: TGroupBox
      Left = 304
      Top = 424
      Width = 129
      Height = 150
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 6
      object Label33: TLabel
        Left = 88
        Top = 24
        Width = 30
        Height = 25
        Alignment = taCenter
        AutoSize = False
        Caption = 'O'
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'Courier New'
        Font.Style = [fsBold]
        ParentColor = False
        ParentFont = False
      end
      object Shape10: TShape
        Left = 88
        Top = 82
        Width = 30
        Height = 20
        Shape = stCircle
      end
      object Button23: TButton
        Left = 8
        Top = 64
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = Button23Click
      end
      object Button24: TButton
        Left = 8
        Top = 104
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button24Click
      end
      object Edit33: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        Text = 'OUT4'
        OnChange = Edit33Change
      end
    end
    object GroupBox35: TGroupBox
      Left = 448
      Top = 424
      Width = 129
      Height = 150
      Caption = 'LEDBI, OUT2, OUT3,...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 7
      object Label34: TLabel
        Left = 88
        Top = 24
        Width = 30
        Height = 25
        Alignment = taCenter
        AutoSize = False
        Caption = 'O'
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'Courier New'
        Font.Style = [fsBold]
        ParentColor = False
        ParentFont = False
      end
      object Shape11: TShape
        Left = 88
        Top = 82
        Width = 30
        Height = 20
        Shape = stCircle
      end
      object Button25: TButton
        Left = 8
        Top = 64
        Width = 65
        Height = 25
        Caption = 'On'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = Button25Click
      end
      object Button26: TButton
        Left = 8
        Top = 104
        Width = 65
        Height = 25
        Caption = 'Off'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Times New Roman'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = Button26Click
      end
      object Edit34: TEdit
        Left = 8
        Top = 24
        Width = 65
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        Text = 'OUT5'
        OnChange = Edit34Change
      end
    end
  end
  object GroupBox21: TGroupBox
    Left = 56
    Top = 336
    Width = 569
    Height = 241
    Caption = 'int i0...i6 '
    Color = clSilver
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    TabOrder = 19
  end
  object GroupBox20: TGroupBox
    Left = 56
    Top = 584
    Width = 569
    Height = 161
    Caption = 'double f0...f5 '
    Color = clSilver
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    TabOrder = 18
  end
  object GroupBox1: TGroupBox
    Left = 56
    Top = 64
    Width = 393
    Height = 81
    Caption = 'ComPort'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 0
    object ComLed1: TComLed
      Left = 240
      Top = 32
      Width = 25
      Height = 25
      ComPort = ComPort1
      LedSignal = lsConn
      Kind = lkRedLight
    end
    object Button1: TButton
      Left = 24
      Top = 24
      Width = 81
      Height = 41
      Caption = 'Connect'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = Button1Click
    end
    object Button2: TButton
      Left = 128
      Top = 24
      Width = 81
      Height = 41
      Caption = 'Disconnect'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = Button2Click
    end
    object Button5: TButton
      Left = 288
      Top = 24
      Width = 83
      Height = 41
      Caption = 'Quit'
      TabOrder = 2
      OnClick = Button5Click
    end
  end
  object GroupBox2: TGroupBox
    Left = 656
    Top = 200
    Width = 561
    Height = 55
    Caption = 'LEDBI, OUT2, OUT3, OUT4...'
    TabOrder = 1
    object Label1: TLabel
      Left = 464
      Top = 16
      Width = 48
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Shape1: TShape
      Left = 528
      Top = 16
      Width = 20
      Height = 25
      Shape = stCircle
    end
    object Button3: TButton
      Left = 112
      Top = 16
      Width = 50
      Height = 25
      Caption = 'Off'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 176
      Top = 16
      Width = 50
      Height = 25
      Caption = 'On'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = Button4Click
    end
    object ScrollBar1: TScrollBar
      Left = 248
      Top = 16
      Width = 201
      Height = 25
      LargeChange = 10
      PageSize = 0
      TabOrder = 2
      OnChange = ScrollBar1Change
    end
    object Edit1: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      Text = 'OUT9'
      OnChange = Edit1Change
    end
  end
  object GroupBox3: TGroupBox
    Left = 72
    Top = 360
    Width = 170
    Height = 60
    Caption = 'var i0'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    object Label3: TLabel
      Left = 80
      Top = 24
      Width = 81
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit3: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i0'
      OnChange = Edit3Change
    end
  end
  object GroupBox4: TGroupBox
    Left = 256
    Top = 360
    Width = 170
    Height = 60
    Caption = 'var i1'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    object Label4: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit4: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i1'
      OnChange = Edit4Change
    end
  end
  object GroupBox5: TGroupBox
    Left = 440
    Top = 360
    Width = 170
    Height = 60
    Caption = 'var i2'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 4
    object Label5: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit5: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i2'
      OnChange = Edit5Change
    end
  end
  object Edit2: TEdit
    Left = 896
    Top = 8
    Width = 121
    Height = 27
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    Text = 'Test 109'
  end
  object GroupBox6: TGroupBox
    Left = 72
    Top = 424
    Width = 170
    Height = 60
    Caption = 'var i3'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 6
    object Label6: TLabel
      Left = 80
      Top = 24
      Width = 81
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit6: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i3'
      OnChange = Edit6Change
    end
  end
  object GroupBox7: TGroupBox
    Left = 256
    Top = 424
    Width = 170
    Height = 60
    Caption = 'var i4'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 7
    object Label7: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit7: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i4'
      OnChange = Edit7Change
    end
  end
  object GroupBox8: TGroupBox
    Left = 440
    Top = 424
    Width = 169
    Height = 60
    Caption = 'var i5'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    TabOrder = 8
    object Label8: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit8: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i5'
      OnChange = Edit8Change
    end
  end
  object GroupBox9: TGroupBox
    Left = 72
    Top = 488
    Width = 537
    Height = 73
    Caption = 'var i6 => b[0]...b[15]'
    TabOrder = 9
    object Label9: TLabel
      Left = 80
      Top = 24
      Width = 81
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Label10: TLabel
      Left = 192
      Top = 8
      Width = 337
      Height = 25
      AutoSize = False
      BiDiMode = bdRightToLeft
      Caption = '15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0'
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Courier New'
      Font.Style = []
      ParentBiDiMode = False
      ParentColor = False
      ParentFont = False
    end
    object Label11: TLabel
      Left = 192
      Top = 32
      Width = 337
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit9: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'i6'
      OnChange = Edit9Change
    end
  end
  object GroupBox12: TGroupBox
    Left = 72
    Top = 608
    Width = 170
    Height = 60
    Caption = 'var f0'
    TabOrder = 10
    object Label12: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit12: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 24
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'f0'
      OnChange = Edit12Change
    end
  end
  object GroupBox13: TGroupBox
    Left = 256
    Top = 608
    Width = 170
    Height = 60
    Caption = 'var f1'
    TabOrder = 11
    object Label13: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit13: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 24
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'f1'
      OnChange = Edit13Change
    end
  end
  object GroupBox14: TGroupBox
    Left = 440
    Top = 608
    Width = 170
    Height = 60
    Caption = 'var f2'
    TabOrder = 12
    object Label14: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit14: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 24
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'f2'
      OnChange = Edit14Change
    end
  end
  object GroupBox15: TGroupBox
    Left = 72
    Top = 672
    Width = 170
    Height = 60
    Caption = 'var f3'
    TabOrder = 13
    object Label15: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit15: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 24
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'f3'
      OnChange = Edit15Change
    end
  end
  object GroupBox16: TGroupBox
    Left = 256
    Top = 672
    Width = 170
    Height = 60
    Caption = 'var f4'
    TabOrder = 14
    object Label16: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit16: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 24
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'f4'
      OnChange = Edit16Change
    end
  end
  object GroupBox17: TGroupBox
    Left = 440
    Top = 672
    Width = 170
    Height = 60
    Caption = 'var f5'
    TabOrder = 15
    object Label17: TLabel
      Left = 80
      Top = 24
      Width = 80
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Edit17: TEdit
      Left = 8
      Top = 24
      Width = 60
      Height = 24
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'f5'
      OnChange = Edit17Change
    end
  end
  object GroupBox18: TGroupBox
    Left = 656
    Top = 264
    Width = 561
    Height = 55
    Caption = 'LEDBI, OUT2, OUT3, OUT4...'
    TabOrder = 16
    object Label18: TLabel
      Left = 464
      Top = 16
      Width = 48
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Shape2: TShape
      Left = 528
      Top = 16
      Width = 20
      Height = 25
      Shape = stCircle
    end
    object Button6: TButton
      Left = 112
      Top = 16
      Width = 50
      Height = 25
      Caption = 'Off'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = Button6Click
    end
    object Button7: TButton
      Left = 176
      Top = 16
      Width = 50
      Height = 25
      Caption = 'On'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = Button7Click
    end
    object ScrollBar2: TScrollBar
      Left = 248
      Top = 16
      Width = 201
      Height = 25
      LargeChange = 10
      PageSize = 0
      TabOrder = 2
      OnChange = ScrollBar2Change
    end
    object Edit18: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      Text = 'OUT10'
      OnChange = Edit18Change
    end
  end
  object GroupBox19: TGroupBox
    Left = 656
    Top = 328
    Width = 561
    Height = 55
    Caption = 'LEDBI, OUT2, OUT3, OUT4...'
    TabOrder = 17
    object Label19: TLabel
      Left = 464
      Top = 16
      Width = 48
      Height = 25
      AutoSize = False
      Color = clWhite
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Shape3: TShape
      Left = 528
      Top = 16
      Width = 20
      Height = 25
      Shape = stCircle
    end
    object Edit19: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 25
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -15
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'OUT11'
      OnChange = Edit19Change
    end
    object Button8: TButton
      Left = 112
      Top = 16
      Width = 50
      Height = 25
      Caption = 'Off'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = Button8Click
    end
    object Button9: TButton
      Left = 176
      Top = 16
      Width = 50
      Height = 25
      Caption = 'On'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      OnClick = Button9Click
    end
    object ScrollBar3: TScrollBar
      Left = 248
      Top = 16
      Width = 201
      Height = 25
      LargeChange = 10
      PageSize = 0
      TabOrder = 3
      OnChange = ScrollBar3Change
    end
  end
  object GroupBox22: TGroupBox
    Left = 56
    Top = 168
    Width = 569
    Height = 161
    Caption = 'a1...a5=analogRead(A1...A11)'
    Color = clSilver
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    TabOrder = 20
    object GroupBox23: TGroupBox
      Left = 16
      Top = 24
      Width = 170
      Height = 60
      Caption = 'var a0'
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      TabOrder = 0
      object Label20: TLabel
        Left = 72
        Top = 24
        Width = 80
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Edit20: TEdit
        Left = 8
        Top = 24
        Width = 60
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'A0'
        OnChange = Edit20Change
      end
    end
    object GroupBox24: TGroupBox
      Left = 200
      Top = 24
      Width = 170
      Height = 60
      Caption = 'var a1'
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      TabOrder = 1
      object Label21: TLabel
        Left = 80
        Top = 24
        Width = 80
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Edit21: TEdit
        Left = 8
        Top = 24
        Width = 60
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'A1'
        OnChange = Edit21Change
      end
    end
    object GroupBox25: TGroupBox
      Left = 384
      Top = 24
      Width = 170
      Height = 60
      Caption = 'var a2'
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      TabOrder = 2
      object Label22: TLabel
        Left = 80
        Top = 24
        Width = 80
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Edit22: TEdit
        Left = 8
        Top = 24
        Width = 60
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'A2'
        OnChange = Edit22Change
      end
    end
    object GroupBox26: TGroupBox
      Left = 16
      Top = 88
      Width = 170
      Height = 60
      Caption = 'var a3'
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      TabOrder = 3
      object Label23: TLabel
        Left = 80
        Top = 24
        Width = 80
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Edit23: TEdit
        Left = 8
        Top = 24
        Width = 60
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'A3'
        OnChange = Edit23Change
      end
    end
    object GroupBox27: TGroupBox
      Left = 200
      Top = 88
      Width = 170
      Height = 60
      Caption = 'var a4'
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      TabOrder = 4
      object Label24: TLabel
        Left = 80
        Top = 24
        Width = 80
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Edit24: TEdit
        Left = 8
        Top = 24
        Width = 60
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'A4'
        OnChange = Edit24Change
      end
    end
    object GroupBox28: TGroupBox
      Left = 384
      Top = 88
      Width = 170
      Height = 60
      Caption = 'var a5'
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Times New Roman'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      TabOrder = 5
      object Label25: TLabel
        Left = 80
        Top = 24
        Width = 80
        Height = 25
        AutoSize = False
        Color = clWhite
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentColor = False
        ParentFont = False
      end
      object Edit25: TEdit
        Left = 8
        Top = 24
        Width = 60
        Height = 25
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'A5'
        OnChange = Edit25Change
      end
    end
  end
  object Button10: TButton
    Left = 536
    Top = 88
    Width = 81
    Height = 41
    TabOrder = 22
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
    OnException = ComPort1Exception
    Left = 328
    Top = 24
  end
end
