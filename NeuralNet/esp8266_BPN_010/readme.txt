Ausgehend von 
http://robotics.hobbizine.com/arduinoann.html
habe ich den Artificial Neural Net Code sumgeschrieben und erweitert.

Als MCU ist mindestens ein ESP8266 notwendig.

Topologie wie bisher, nur (etwas) vergr��ert:
Als Inputs k�nnen jetzt 120 Bit-Werte eingegeben werden (0/1),
Die Inputs werden direkt 1:1 der Inputschickt zugeordnet (InputLayer= IL).
Dann wird der Ausgang jedes IL-Neurons an jedes Neuron der Zwischenschicht geleitet (HiddenLayer=HL, mit derzeit 14 Neuronen zu je 120 Eing�ngen; d.h. 120x14=1680 neuronale Verkn�pfungen),
jedes HL Neuron hat wieder nur 1 Ausgang.
Dann wird der Ausgang jedes HL-Neurons an die Eing�nge von jedem Output-Neuron geleitet (OutputLayer=OL, mit 10 Neuronen zu je 14 Eing�ngen; d.h. 14x10= 140 neuronale Verkn�pfungen):
jedes OL-Neuron hat dann wieder genau 1 Ausgang, alle 10 zusammen bilden dann den endg�ltigen Netz-Output.

Jedes der 120-bit Input-Muster erzeugt damit je 1 Output-Muster von je 10 bit L�nge.
Es k�nnen z.Zt 100 verschiedene Input-Output-Muster trainiert werden (kann noch erh�ht werden).
Ebenfalls k�nnen die Neuronen-Zahlen der einzenen Schichten (Layer) noch ver�ndert werden.

Wegen des nun (etwas) gr��eren Netzes mussten ein paar Stellvariablen f�r den Lernalgorithmus leicht ge�ndert werden:
float LearningRate = 0.1; // 0.3
float Momentum = 0.6; // 0.8
Damit wird das Risiko, dass das Netz nicht konvergiert (weil es oszilliert oder weil es in lokalen Minima gefangen ist) etwas kleiner (aber auch nicht 100%ig verhindert).
Der Lernerfolg wird ebenfalls intern beobachtet, und wenn das Netz nicht konvergieren sollte, dann werden die Neuronen neu randomisiert initialsiert und automatisch neu gestartet.

Alle Inputs und alle Lernmuster sind noch hard-coded, was bei 100x120 Inputs etwas aufw�ndig und un�bersichtlich ist.
Verzichtet man zu Testzwecken auf eine vollst�ndige Initialisierung aller je 120 Inputs, reicht es, nur die ersten signifikanten Bits einzugeben (d.h. z.B. nur die ersten 10) und die letzten, undefinierten, ganz wegzulassen
- optimal ist das ntl nicht, aber meine Zahn�rztin sagt, es geht.

Momentan sind die Input-Muster 0-9 bespielhaft vorbelegt mit den Werten des Original-Codes (Kodierungen f�r 0-9) und die Muster 20-29 zus�tzlich mit "LCD-Pixel-Mustern" f�r die Zeichen "0" bis "9", und die Ziffern werden dann als Output mit ihrem Zahlenwert 0-9 (bin�r) trainiert:
daher ergeben momentan die Muster 0/20 als Output 0, 1/21 als Output 1, 2/22 als Output 2, usw...


Im seriellen Monitor (115200 baud) k�nnen die Input/Target/Output-Werte und der Lernfortschritt beobachtet werden.
Geduld w�hrend des Trainings ist n�tig, es kann (je nach Mustern) etliche Stunden dauern...

Testl�ufe mit Input-Mustern sind jetzt ebenfalls m�glich, und per #define DEBUG kann das Lerntraining zu Debug-Zwecken abgek�rzt werden.

Eine M�glichkeit zur Werteeingabe w�hrend der Laufzeit will ich evt noch irgendwann zus�tzlich versuchen einzubauen.


share and enjoy!