    <b>FLOOD DETECTION AND AVERSION SYSTEM</b> <br>
The idea<br>
1. Read parameters from several sensors i.e. soil moisture sensor, water level sensor, and rain drop sensor <br>
2. Compute the level of imminent flooding<br>
<ul>
  <li> Level 0: Warning</li>
  <li> Level 1: Critical</li>
  <li> Level 2: Healthy (No danger)</li>
<ul><br>
3. Depending on the level of imminence, open a valve system to drain the water, while doing some displays. Repeat till no danger<br>
<ul>
  <li> Level 0: Warning</li>
	<ul>
	<li>Light blue LED</li>
	<li>Print, "WARNING AVERTING"</li>
	<li>Play repeated beep on buzzer</li>
	<li>Open valve partially (90deg on Servo)</li>
	</ul>
  <li> Level 1: Danger</li>
	<ul>
	<li>Light edLED</li>
	<li>Print, "DANGER REACTING"</li>
	<li>Play siren on buzzer</li>
	<li>Open valve fully (180deg on Servo)</li>
	</ul>
  <li> Level 2: Healthy (No danger)</li>
	<ul>
	<li>Light green LED</li>
	<li>Print, "HEALTHY CHILLING"</li>
	<li>Silent buzzer</li>
	<li>Closed valve (0deg on Servo)</li>
	</ul>
<ul><br>
4. Parts needed:
<ul>
  <li>Servo motor</li>
  <li>Water level sensor</li>
  <li>Raindrop sensor</li>
  <li>Soil moisture sensor</li>
  <li>LCD</li>
  <li>RGB LED</li>
  <li>Buzzer</li>
</ul>