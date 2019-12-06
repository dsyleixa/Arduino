# Arduino


ButtonClass tButton

Instances: e.g.


tButton mybtn;



// and perhaps more:


tButton mybtn1;


tButton mybtn2;


// etc


Init:

mybtn.init(pin_number,  INPUT/INPUT_PULLUP)
- or -
mybtn.init(pin_number,  INPUT/INPUT_PULLUP,  min_press_ms)
(for INPUT_PULLUP mode, the press action is inverted automatically)
(min_press_ms default: 60ms)



to retrieve/refresh button click  states: 
call repeatedly


mybtn.click() 
alias:  mybtn.state() 


it returns a int8_t value 0, 1, 2, or 3:


0: not pressed

1: 1 short click (default minimum duration = 60ms)

2: double click = 2 quick short clicks (default: within <= 150ms)

3: a long press (minimum duration= 300ms)