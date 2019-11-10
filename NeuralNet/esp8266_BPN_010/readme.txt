Ausgehend von 
http://robotics.hobbizine.com/arduinoann.html
www.cs.bham.ac.uk/~jxb/NN/nn.html
habe ich den Artificial Neural Net Code umgeschrieben und erweitert.

Als MCU ist mindestens ein ESP8266 notwendig.

Topologie wie bisher, nur (etwas) vergrößert:
Es können jetzt 120 Input-Bits eingegeben werden (1 Bit/Input).
Die Inputs werden direkt 1:1 der Inputschicht zugeordnet (InputLayer= IL).
Dann wird der Ausgang jedes IL-Neurons an jedes Neuron der Zwischenschicht geleitet (HiddenLayer=HL),
jedes HL Neuron hat wieder nur 1 Ausgang.
Dann wird der Ausgang jedes HL-Neurons an die Eingänge von jedem der 10 Output-Neurons geleitet (OutputLayer=OL)
jedes OL-Neuron hat dann wieder genau einen 1-Bit-Ausgang, alle zusammen bilden dann den endgültigen Netz-Output.

Jedes der 120-bit Input-Muster erzeugt damit je 1 Output-Muster von je 10 bit Länge.
Es können z.Zt 100 verschiedene Input-Output-Muster trainiert werden (kann noch erhöht werden).
Ebenfalls können die Neuronen-Zahlen der einzenen Schichten (Layer) noch verändert werden.

Der Lernerfolg wird ebenfalls intern beobachtet, und wenn das Netz nicht konvergieren sollte, dann werden die Neuronen neu randomisiert initialsiert und automatisch neu gestartet.

Alle Inputs und alle Lernmuster sind noch hard-coded, was bei sehr vielen Input-Mustern etwas aufwändig und unübersichtlich ist.
Verzichtet man zu Testzwecken auf eine vollständige Initialisierung aller je 120 Inputs, reicht es, nur die ersten signifikanten Bits einzugeben (d.h. z.B. nur die ersten 10) und die letzten, undefinierten, ganz wegzulassen
- optimal ist das ntl nicht...

Momentan sind die Input-Muster 0-9 bespielhaft vorbelegt mit den Werten des Original-Codes (Kodierungen für 0-9) und die Muster 20-29 zusätzlich mit "LCD-Pixel-Mustern" für die Zeichen "0" bis "9", und die Ziffern werden dann als Output mit ihrem Zahlenwert 0-9 (binär) trainiert:
daher ergeben momentan die Muster 0/20 als Output 0, 1/21 als Output 1, 2/22 als Output 2, usw...


Im seriellen Monitor (115200 baud) können die Input/Target/Output-Werte und der Lernfortschritt beobachtet werden.
Geduld während des Trainings ist nötig, es kann (je nach Mustern) etliche Stunden dauern...

Testläufe mit Input-Mustern sind jetzt ebenfalls möglich, und per #define DEBUG kann das Lerntraining zu Debug-Zwecken abgekürzt werden.



References, Resources:
Hobbizine: http://robotics.hobbizine.com/arduinoann.html
John Bullinaria: www.cs.bham.ac.uk/~jxb/NN/nn.html. 
additional resources at TEK271.com: http://www.tek271.com/?about=docs/neuralNet/IntoToNeuralNets.html 
Robotics Society of Southern California: http://www.rssc.org/content/introduction-neural-nets-part-1. 
