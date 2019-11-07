Ausgehend von 
http://robotics.hobbizine.com/arduinoann.html
habe ich den Artificial Neural Net Code sumgeschrieben und erweitert.

Als MCU ist mindestens ein ESP8266 notwendig.

Topologie wie bisher, nur (etwas) vergrößert:
Als Inputs können jetzt 120 Bit-Werte eingegeben werden (0/1),
Die Inputs werden direkt 1:1 der Inputschickt zugeordnet (InputLayer= IL).
Dann wird der Ausgang jedes IL-Neurons an jedes Neuron der Zwischenschicht geleitet (HiddenLayer=HL, mit derzeit 14 Neuronen zu je 120 Eingängen; d.h. 120x14=1680 neuronale Verknüpfungen),
jedes HL Neuron hat wieder nur 1 Ausgang.
Dann wird der Ausgang jedes HL-Neurons an die Eingänge von jedem Output-Neuron geleitet (OutputLayer=OL, mit 10 Neuronen zu je 14 Eingängen; d.h. 14x10= 140 neuronale Verknüpfungen):
jedes OL-Neuron hat dann wieder genau 1 Ausgang, alle 10 zusammen bilden dann den endgültigen Netz-Output.

Jedes der 120-bit Input-Muster erzeugt damit je 1 Output-Muster von je 10 bit Länge.
Es können z.Zt 100 verschiedene Input-Output-Muster trainiert werden (kann noch erhöht werden).
Ebenfalls können die Neuronen-Zahlen der einzenen Schichten (Layer) noch verändert werden.

Wegen des nun (etwas) größeren Netzes mussten ein paar Stellvariablen für den Lernalgorithmus leicht geändert werden:
float LearningRate = 0.1; // 0.3
float Momentum = 0.6; // 0.8
Damit wird das Risiko, dass das Netz nicht konvergiert (weil es oszilliert oder weil es in lokalen Minima gefangen ist) etwas kleiner (aber auch nicht 100%ig verhindert).
Der Lernerfolg wird ebenfalls intern beobachtet, und wenn das Netz nicht konvergieren sollte, dann werden die Neuronen neu randomisiert initialsiert und automatisch neu gestartet.

Alle Inputs und alle Lernmuster sind noch hard-coded, was bei 100x120 Inputs etwas aufwändig und unübersichtlich ist.
Verzichtet man zu Testzwecken auf eine vollständige Initialisierung aller je 120 Inputs, reicht es, nur die ersten signifikanten Bits einzugeben (d.h. z.B. nur die ersten 10) und die letzten, undefinierten, ganz wegzulassen
- optimal ist das ntl nicht, aber meine Zahnärztin sagt, es geht.

Momentan sind die Input-Muster 0-9 bespielhaft vorbelegt mit den Werten des Original-Codes (Kodierungen für 0-9) und die Muster 20-29 zusätzlich mit "LCD-Pixel-Mustern" für die Zeichen "0" bis "9", und die Ziffern werden dann als Output mit ihrem Zahlenwert 0-9 (binär) trainiert:
daher ergeben momentan die Muster 0/20 als Output 0, 1/21 als Output 1, 2/22 als Output 2, usw...


Im seriellen Monitor (115200 baud) können die Input/Target/Output-Werte und der Lernfortschritt beobachtet werden.
Geduld während des Trainings ist nötig, es kann (je nach Mustern) etliche Stunden dauern...

Testläufe mit Input-Mustern sind jetzt ebenfalls möglich, und per #define DEBUG kann das Lerntraining zu Debug-Zwecken abgekürzt werden.

Eine Möglichkeit zur Werteeingabe während der Laufzeit will ich evt noch irgendwann zusätzlich versuchen einzubauen.


share and enjoy!