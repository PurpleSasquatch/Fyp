/*
   Code for the MIDI Glove portion of my FYP that generates the MIDI commmands and controls
   based on the finger input
   this version of the code is designed for creating the MIDI commmands for note on, note off
   and pitch modulation

   Author Fionn Fitzgerald
   26/02/2019
*/

///////////////////////////////////////////////////////////////////////////////////////////////
//comparision value for last value to be read from fingers initilised outside the
//main loop to stop it being rewritten each loop
///////////////////////////////////////////////////////////////////////////////////////////////
int lastGray = 0;

///////////////////////////////////////////////////////////////////////////////////////////////
//Serial1 is the baud rate for MIDI connections
//the HC-06 has been modified with the AT commands neccassary to change its Baud rate
//Serial is the serial moniter baud rate used for debugging
///////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial1.begin(38400);
  Serial.begin(38400);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  pinMode(1, INPUT);
}

void loop() {


  //read finger values and shift 2 to the right to change max from 1024 to 127
  //from arduino values to MIDI values
  int middle = analogRead(A2) >> 2;
  int ring = analogRead(A3) >> 2;
  int pinkie = analogRead(A4) >> 2;

  int bit1 = 0;
  int bit2 = 0;
  int bit3 = 0;

  int note = 0;
  int noteconst = 0;
  int cmd = 0;
  int loudest = 0;
  int transpose = 0;

  //small delay to allow user to press multiple fingers
  delay(25);

  if (digitalRead(1) == HIGH)
  {
    transpose = 2;
  }
  else
  {
    transpose = 0;
  }
  //set bits for notes and comparisions
  if (middle > 10)
  {
    bit1 = B001;
  }
  if (ring > 10)
  {
    bit2 = B010;
  }
  if (pinkie > 10)
  {
    bit3 = B100;
  }

  //combine bit patterns to create the code on which the notes will be played
  int grayCode = bit1 + bit2 + bit3;
  Serial.println(grayCode);
  //checks for a change in notes being played or allows the note to ring out
  if (lastGray == grayCode && grayCode != 0) {

    int level = analogRead(A5);
    int mod = 0;// amount of pitch modulation
    //if the wrist flexor is not moved it sends no signal when moved it sends the value to the pitch
    //modulation command
    //the first if loop gives a buffer zone for unintended movement
    //the other if statements change the value to a range from 0 to 127

    if (level < 510 && level > 460)
    {
      mod = 0;
    } else if (level > 510 && level < 646)
    {
      mod = level - 484;
    } else if (level < 460 && level > 334);
    {
      mod = 484 - level;
    }

    sendNote(0xE0, 0, mod);
    //0xC0 channel pressure command
    if (middle > ring)
    {
      if (middle > pinkie)
      {
        loudest = middle;
      } else
      {
        loudest = pinkie;
      }
    } else if (ring > pinkie)
    {
      loudest = ring;
    }
  }
  if (loudest > 10)
  {
    Serial1.write(0xC0);
    Serial1.write(loudest);
  }


  //if note has been changed play new note
  if (lastGray != grayCode)
  {
    //if high finger on set the constat to 12, each MIDI note is equal to the same note an octave lower plus 12
    if (analogRead(A1) > 100)
    {
      noteconst = 12;
    }
    //high finger off++(play note without adding 12(shift up one octave))
    else
    {
      noteconst = 0;
    }
    //when note changes turn off notes to make it easier to play chords and play
    //the notes requiring more than one finger
    turnoffNote();
    cmd = 0x90;
    if (Serial1.read() != -1)
    {
      digitalWrite(8, HIGH);//blue Led means connected

      //updates lastgray
      lastGray = grayCode;
      Serial.println("last="+lastGray);
      // if Chord Finger on go to case switch structure to play chord
      if (analogRead(A0) > 100)
      {
        switch (grayCode)
        {
          //defualt case for debugging purposes
          default:
            Serial1.write(cmd);
            Serial1.write(30);
            Serial1.write(1);
            break;
          //when all fingers released call turnoffNote function to clear notes being played
          case 0:
            turnoffNote();
            break;
          //calls the sendchord function with the name of each chord
          case 1:
            sendChord("C", noteconst, transpose);
            break;

          case 2:
            sendChord("D", noteconst, transpose);
            break;

          case 3:
            sendChord("E", noteconst, transpose);
            break;

          case 4:
            sendChord("F", noteconst, transpose);
            break;

          case 5:
            sendChord("G", noteconst, transpose);
            break;

          case 6:
            sendChord("A", noteconst, transpose);
            break;

          case 7:
            sendChord("B", noteconst, transpose);
            break;
        }
      }
      //else prevents notes from being played while the chord is playing
      else {

        switch (grayCode)
        {
          default:
            Serial1.write(cmd);
            Serial1.write(20);
            Serial1.write(1);
            break;

          //turns off all notes when nothing is being pressed
          case 0:
            turnoffNote();
            break;

          //calls noteon function with the note value either as the base value or shifted an octave up
          case 1:
            note = noteconst + 60;
            sendNote(cmd, note, 127);
            break;

          case 2:
            note = noteconst + 62;
            sendNote(cmd, note, 127);
            break;

          case 3:
            note = noteconst + 63;
            sendNote(cmd, note, 127);
            break;

          case 4:
            note = noteconst + 65;
            sendNote(cmd, note, 127);
            break;

          case 5:
            note = noteconst + 67;
            sendNote(cmd, note, 127);
            break;

          case 6:
            note = noteconst + 68;
            sendNote(cmd, note, 127);
            break;

          case 7:
            note = noteconst + 70;
            sendNote(cmd, note, 127);
            break;
        }
      }
    }
    else
    {
      //turns off connected LED if data being recieved on the HC-06 is -1(non-connect signal)
      turnoffNote();
    }
  }

}
///////////////////////////////////////////////////////////////////////////////////////////////
//send note command sends note with the note value and the volume
//command cmd is the MIDI note on command
///////////////////////////////////////////////////////////////////////////////////////////////
void sendNote(int cmd, int note, int volume)
{
  Serial1.write(cmd);
  Serial1.write(note);
  Serial1.write(volume);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//turn off function cycles through all notes and sends a note off MIDI command(0x80)
///////////////////////////////////////////////////////////////////////////////////////////////
void turnoffNote()
{
  int noteoff = 48;
  while (noteoff < 85)
  {
    sendNote(0x80, noteoff, 127);
    noteoff++;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//send chord command sends the three notes associated with a chord based on the integer value
//assigned to each chord name in the key of C
///////////////////////////////////////////////////////////////////////////////////////////////
void sendChord(String chord, int noteconst, int transpose)
{
  int chordInt = 0;
  //C Major
  if (chord == "C")
  {
    chordInt = 1;
  }

  //D Minor
  else if (chord == "D")
  {
    chordInt = 2;
  }

  //E Minor
  else if (chord == "E")
  {
    chordInt = 3;
  }

  //F Major
  else if (chord == "F")
  {
    chordInt = 4;
  }

  //G Major
  else if (chord == "G")
  {
    chordInt = 5;
  }

  //A Minor
  else if (chord == "A")
  {
    chordInt = 6;
  }

  //B Diminished
  else if (chord == "B")
  {
    chordInt = 7;
  }
  switch (chordInt)
  {
    case 1:
      sendNote(0x90, 48 + noteconst + transpose, 127);
      sendNote(0x90, 51 + noteconst + transpose, 127);
      sendNote(0x90, 55 + noteconst + transpose, 127);
      break;

    case 2:
      sendNote(0x90, 50 + noteconst + transpose, 127);
      sendNote(0x90, 53 + noteconst + transpose, 127);
      sendNote(0x90, 56 + noteconst + transpose, 127);
      break;

    case 3:
      sendNote(0x90, 51 + noteconst + transpose, 127);
      sendNote(0x90, 55 + noteconst + transpose, 127);
      sendNote(0x90, 58 + noteconst + transpose, 127);
      break;

    case 4:
      sendNote(0x90, 53 + noteconst + transpose, 127);
      sendNote(0x90, 56 + noteconst + transpose, 127);
      sendNote(0x90, 60 + noteconst + transpose, 127);
      break;

    case 5:
      sendNote(0x90, 55 + noteconst + transpose, 127);
      sendNote(0x90, 58 + noteconst + transpose, 127);
      sendNote(0x90, 62 + noteconst + transpose, 127);
      break;

    case 6:
      sendNote(0x90, 56 + noteconst + transpose, 127);
      sendNote(0x90, 60 + noteconst + transpose, 127);
      sendNote(0x90, 63 + noteconst + transpose, 127);
      break;

    case 7:
      sendNote(0x90, 58 + noteconst + transpose, 127);
      sendNote(0x90, 62 + noteconst + transpose, 127);
      sendNote(0x90, 65 + noteconst + transpose, 127);
      break;
  }
}
