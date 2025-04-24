
// This function compares LCDLines to known lines
// If there is an error of 1 or 2 characters, known line substituted.
// If error of more than 2 characters, orignal line returned.

void LCDCheck()
{
// for receicing data from sniffer and sending data to Blynk LCD
// correct lines used for testing
// 0 is line 1, 1 is line 2  
int numCorrectLines = 168; // update as needed for changes
int numSystemLines = 96;
const char correctLines[168][20] = 
{
// total lines 17-184 168 lines even line 1, odd line 2
// update int num CorrectLines below as needed for changes
{"System is       \0"}, // System messages 96 lines 17-112 - diff plus 1 is number of actual lines or 0-95
{"Ready to Arm    \0"}, // User zone labels 72 lines 113 to 184 or >= 96
{"System is       \0"}, //  **** update above as well ****
{"Ready to Arm  <>\0"}, // added to enable proper decoding - appears to be transient state
{"  Date     Time \0"}, 
{"JAN xx/xx yy:yyz\0"},
{"  Temp  tt F    \0"}, // Temp is displayed with the outside siren
{"JAN xx/xx yy:yyz\0"},
{"Secure System   \0"},
{"Before Arming <>\0"},
{"System          \0"},  // System conflicts with "System is" above as just 2 errors - added special case
{"is in Alarm   <>\0"}, 
{"Scroll to View  \0"},
{"Alarm Memory    \0"}, 
{"System Disarmed \0"},
{"No Alarm Memory \0"},  
{"Scroll to View  \0"},
{"Open Zones    <>\0"},
{"Press (*) for <>\0"},
{"Zone Bypass     \0"},
{"Scroll to     <>\0"},  
{"Bypass Zones    \0"},
{"Press (*) for <>\0"},
{"Bypass Options  \0"},
{"Press (#)     <>\0"},
{"to Exit         \0"},
{"Press (*) for <>\0"},
{"System Troubles \0"},
{"There are No  <>\0"},
{"System Troubles \0"},
{"There are No  <>\0"},
{"Zone Faults     \0"},
{"Press (*) for <>\0"},
{"Alarm Memory    \0"}, 
{"Press (*) for <>\0"},
{"Door Chime      \0"},
{"Press (*) for <>\0"},
{"Access Codes    \0"},
{"Press (*) for <>\0"},
{"User Functions  \0"},
{"Press (*) for <>\0"},
{"No-Entry Arming \0"},
{"Press (*) for <>\0"},
{"Quick Arming    \0"},
{"Press (#)     <>\0"},
{"to Exit         \0"}, 
{"Exit Delay in   \0"},
{"Progress        \0"},
{"System is Armed \0"},
{"in Stay Mode  <>\0"},
{"* Warrning *    \0"},
{"Bypass Active <>\0"},
{"(*1) to Arm     \0"},   
{"Interior      <>\0"},
{"Interior Has    \0"}, 
{"Been Armed      \0"},
{"Door Chime      \0"}, 
{"Feature ON      \0"}, 
{"Door Chime      \0"},
{"Feature OFF     \0"}, 
{"Enter Master    \0"},
{"Access Codes    \0"},
{"Select Option <>\0"},
{"Event Buffer    \0"},
{"Select Option <>\0"},
{"System Test     \0"},
{"Select Option <>\0"},
{"Time and Date   \0"},
{"Select Option <>\0"},
{"Late Open Ctrl  \0"},
{"Select Option <>\0"},
{"Late Open Time  \0"},
{"Select Option <>\0"},
{"System Serv/DLS \0"},
{"Select Option <>\0"},
{"User Callup     \0"},
{"Select Option <>\0"},
{"Walk Test       \0"},
{"Select Option <>\0"},
{"Bright Control  \0"},
{"Select Option <>\0"},
{"Contrast Control\0"},
{"Select Option <>\0"},
{"Buzzer Control  \0"},
{"Brightness      \0"},
{"Level   2     <>\0"},
{"Contrast        \0"},
{"Level   9     <>\0"},
{"Buzzer          \0"},
{"Level   07    <>\0"},
{"Enter Section   \0"},
{"                \0"},
{"Toggle Option   \0"},
{"                \0"},
{"Enter Your      \0"},
{"Installers Code \0"},
{"RE508X T1 T2    \0"}, // User zone lable lines
{"                \0"}, // User zone lines have srolls on top for bypassing zones and on bottom instead for open zones - check and fix below
{"Main Garage     \0"},
{"  Doors         \0"},  // spaces to enable proper decoding re Door
{"Main Garage     \0"},
{"Storage Door    \0"},
{"Bedroom 4 Door  \0"},  // single line to enable proper decoding re Bedroom 4 Door
{"                \0"},  
{"Bedroom 3       \0"},  
{"Door            \0"},  
{"Stairs Door     \0"},    
{"                \0"},
{"Family Room     \0"},
{"Single Door     \0"},
{"Family Room     \0"},
{"Double Doors    \0"},
{"Studio Kitchen  \0"},
{"Door            \0"},
{"RE508X   T3 T4  \0"},  // T4 was conflicting with Date Time check - moved T3 T4 right one space
{"                \0"},
{"Studio Garage   \0"},
{"  Doors         \0"},
{"Front Door      \0"},
{"                \0"},
{"Main Garage     \0"},
{"Door            \0"},
{"Laundry Room    \0"},
{"Delay Door      \0"},
{"Laundry Room    \0"},
{"Delay Door      \0"},
{"Hall            \0"},
{"Delay Door      \0"},
{"Studio Door     \0"},
{"                \0"},
{"Zone   17       \0"},
{"Not Used        \0"},
{"Zone    18      \0"},
{"Not Used        \0"},
{"Motion          \0"},
{"Foyer           \0"},
{"Motion          \0"},
{"Lower Stairs    \0"},
{"Motion          \0"},
{"Upper Stairs    \0"},
{"Motion          \0"},
{"Living Room     \0"},  
{"Motion          \0"},
{"Family Room     \0"},
{"Motion          \0"},
{"Library         \0"},
{"Motion          \0"},
{"Media Room      \0"},
{"Motion          \0"},
{"Main Garage     \0"},
{"Motion          \0"},
{"Smart Room      \0"},
{"Motion Studio   \0"},
{"Garage Window   \0"},
{"Motion          \0"},
{"Studio Kitchen  \0"},
{"Motion          \0"},
{"Studio Up       \0"},
{"Motion Studio   \0"},
{"Garage Door     \0"},
{"Smoke           \0"},
{"Lower Stairs    \0"},
{"Smoke           \0"},
{"Main Garage     \0"},
{"Smoke Main      \0"},
{"Garage Storage  \0"},
{"Smoke           \0"},
{"Studio Garage   \0"},
};
// end of correctLines array

/*
// global variables
// for LCD Check function
char displayLine1In[40];
char displayLine1Out[40];
char displayLine2In[40];
char displayLine2Out[40];
char lastDisplayLine1Out[40];
char lastDisplayLine2Out[40];
int errorCntLine1 = 0;
int errorCntLine2 = 0;
*/


strcpy(displayLine1Out, displayLine1In); // default pass through lines if not 16 bytes
strcpy(displayLine2Out, displayLine2In); 

// check if first line is date time
if ((displayLine1Out[2] == 'D') || (displayLine1Out[11] == 'T')){ // just check these two positions
  strcpy(displayLine1Out, "  Date     Time \0"); 
  formatDateTime();  // formats second line
  strcpy(lastDisplayLine1Out, displayLine1Out);  // save lines for possible future use
  strcpy(lastDisplayLine2Out, displayLine2Out);  
  return;
  }

// check if first line is temperature
if ((displayLine1Out[11] == 'F') || (displayLine1Out[10] == 0xBF)){ // just check these two positions
  // 0xBF is the LCD input for a degree sign
  displayLine1Out[0] = ' ';
  displayLine1Out[1] = ' ';
  displayLine1Out[2] = 'T';
  displayLine1Out[3] = 'e';
  displayLine1Out[4] = 'm';
  displayLine1Out[5] = 'p';
  displayLine1Out[6] = ' '; 
  //displayLine1Out[7] = ' '; // 789 is temp 102
  // displayLine1Out[10] = 0xB0; // degree sign extended ascii - does not seem to work on Blynk LCD
  displayLine1Out[10] = ' '; // use space for degree sign
  displayLine1Out[11] = 'F';
  displayLine1Out[12] = ' '; // clear balance of line
  displayLine1Out[13] = ' ';
  displayLine1Out[14] = ' ';
  displayLine1Out[15] = ' ';
  formatDateTime();  // formats second line
  strcpy(lastDisplayLine1Out, displayLine1Out);  // save lines for possible future use
  strcpy(lastDisplayLine2Out, displayLine2Out);  
  return;
  }

// compare lines to correctLines array strings
// line 1 first - every other line
if (strlen(displayLine1In) == 16){  // other lengths returned unchanged
  // to start, clear input lines of invalid characters by substituting spaces
  for (int k = 0; k <= 15; k ++) {
    if (!checkChar(displayLine1In[k])) {displayLine1In[k] = ' ';} 
    }    
  // next compare to correct lines
  for (int i = 0; i <= numCorrectLines; i = i + 2) {
      errorCntLine1 = 0; // reset error count and index for each line check
      indexLine1Out = -1;
      if (i == numCorrectLines) { // no lines found with less than 3 errors, so same line returned
        strcpy(displayLine1Out, displayLine1In); 
        indexLine1Out = -1;
        break;
      } 
    for (int j = 0; j <= 15; j ++) { // compare lines
      if (displayLine1In[j] != correctLines[i][j]) errorCntLine1++;
      if ((i >= numSystemLines) && (j == 13)) {break;}  // skip last two for zone names for bypass open scroll issues addressed below
      }
    if (errorCntLine1 == 0) { // no errors in lines, same line returned
      strcpy(displayLine1Out, displayLine1In);
      indexLine1Out = i;
      break; // break out of loop
      }
    if (errorCntLine1 <= 2) { // 1 or 2 errors in line, correctLines replaces
      strcpy(displayLine1Out, correctLines[i]);
      indexLine1Out = i;
      break;
      }
    // error count >= 3 loops and does not break
    }
  }  

// line 2 next - every other line from 1
if (strlen(displayLine2In) == 16){
  // to start, clear input lines of invalid characters by substituting spaces
  for (int k = 0; k <= 15; k ++) {
    if (!checkChar(displayLine2In[k])) {displayLine2In[k] = ' ';} 
    }    
  for (int i = 1; i <= (numCorrectLines + 1); i = i + 2) {
    errorCntLine2 = 0; // reset error count and index for each line check
    indexLine2Out = -1;
    if (i == (numCorrectLines + 1)) { // no lines found with less than 3 errors, so same line returned
      strcpy(displayLine2Out, displayLine2In); 
      indexLine2Out = -1;
      break;
      } 
    for (int j = 0; j <= 15; j ++) { // compare lines
      if (displayLine2In[j] != correctLines[i][j]) errorCntLine2++;
      if ((i >= numSystemLines) && (j == 13)) {break;}  // skip last two for zone names for bypass open scroll issues addressed below
      }
    if (errorCntLine2 == 0) { // no errors in lines, same line returned
      strcpy(displayLine2Out, displayLine2In); 
      indexLine2Out = i;   
      break;
      } 
    if (errorCntLine2 <= 2) { // 1 or 2 errors in line, correctLines replaces
      strcpy(displayLine2Out, correctLines[i]);
      indexLine2Out = i;
      break;
      }
    // error count >= 3 loops and does not break
    }
  }
// final check for any special cases
// Alarm Bell
if (strcmp(displayLine2Out, "is in Alarm   <>\0") == 0){
  strcpy(displayLine1Out, "System          \0"); // update line 1 for conflict with "System is"
  strcpy(lastDisplayLine1Out, displayLine1Out);  // save updated line 1 for possible future use
  alarmFlagLCD = 1;  // set alarm flag
  lastCommandMillis = currentMillis; // update LCD display time out alarm
  }
if ( (strcmp(displayLine1Out, "System Disarmed \0") == 0) // note line 1 - also this can go by fast between screens
  || (strcmp(displayLine2Out, "Ready to Arm    \0") == 0) ) { 
  //  || (strcmp(displayLine2Out, "Alarm Memory    \0") == 0) ) {// note - this did not work well 
  alarmFlagLCD = 0;  // clear alarm flag
  lastCommandMillis = currentMillis; // update LCD display time out on alarm disable
  }
// Add any <> indicated for bypass or open zone to line 1 or 2 
if (indexLine2Out >= numSystemLines) { // if in zone lines check for scroll marks on input line 1 input
  if ((displayLine2In[14] == '<') || (displayLine2In[15] == '>')){
    displayLine1Out[14] = ' '; // clear line 1 scrolls
    displayLine1Out[15] = ' ';
    displayLine2Out[14] = '<'; // add any missing line 2 scrolls
    displayLine2Out[15] = '>';
    }
  if ((displayLine1In[14] == '<') || (displayLine1In[15] == '>')){
    displayLine2Out[14] = ' '; // clear line 2 scrolls
    displayLine2Out[15] = ' ';
    displayLine1Out[14] = '<'; // add any missing line 1 scrolls
    displayLine1Out[15] = '>';
    }
  }
// Add any B indicated for bypassed zone to line 2, last position
if (indexLine2Out >= numSystemLines) { // if in zone lines pass any B through
  if (displayLine2In[15] == 'B') {displayLine2Out[15] = 'B';}
  }
// Add any ON OFF indicated for Chime in positions 8, 9, 10
if ((displayLine2Out[0] == 'F') && (displayLine2Out[1] == 'e') && (displayLine2Out[2] == 'a')) { // if in Feature chime lines pass through ON OFF
  displayLine2Out[8] = displayLine2In[8];
  displayLine2Out[9] = displayLine2In[9];
  displayLine2Out[10] = displayLine2In[10];
  }
strcpy(lastDisplayLine1Out, displayLine1Out);  // save lines for possible future use
strcpy(lastDisplayLine2Out, displayLine2Out);   
return;
}  /// end of LCDCheck

// Takes line 2 of Date Time display and checks for and corrects errors
// Does not use nano's time to correct errors
// "JAN xx/xx yy:yyz\0"
void formatDateTime()
{
const char month[12][6] = {{"JAN\0"}, {"FEB\0"}, {"MAR\0"}, {"APR\0"}, {"MAY\0"}, {"JUN\0"},
 {"JUL\0"}, {"AUG\0"}, {"SEP\0"}, {"OCT\0"}, {"NOV\0"}, {"DEC\0"}};
char monthStrIn[6] = "";
char monthStrOut[6] = "";
int errorCntMonth = 0;

//format fixed chars
displayLine2Out[6] = '/';
displayLine2Out[9] = ' ';
displayLine2Out[12] = ':';
if ((displayLine2Out[15] != 'a') && (displayLine2Out[15] != 'p')) {displayLine2Out[15] = ' ';}
// save proposed month
monthStrIn[0] = displayLine2In[0];
monthStrIn[1] = displayLine2In[1];
monthStrIn[2] = displayLine2In[2];
monthStrIn[3] = '\0';
for (int i = 0; i <= 12; i++) { // 11 months +1 at end
  if (i == 12) { // no month found with less than 2 errors, so same line returned
    strcpy(monthStrOut, monthStrIn); 
    break;
    }
  if (monthStrIn[0] != month[i][0]) errorCntMonth++;
  if (monthStrIn[1] != month[i][1]) errorCntMonth++; 
  if (monthStrIn[2] != month[i][2]) errorCntMonth++; 
  if (errorCntMonth == 0) {strcpy(monthStrOut, monthStrIn); break;} // no errors in month, same month returned
  if (errorCntMonth == 1) {strcpy(monthStrOut, month[i]); break;} // one error in month, corrected month returned
  if (errorCntMonth >= 2) {errorCntMonth = 0;} // check next month in array
  }
// special case for JAN and JUN
if (strcmp(monthStrOut, month[0]) == 0){ // first month wwhere error would occur
  strcpy(monthStrOut, monthStrIn); // revert if correct month not actually known
  if (monthStrIn[1] == 'A') {strcpy(monthStrOut, month[0]);} // JxN or JxN to JAN?
  if (monthStrIn[1] == 'U') {strcpy(monthStrOut, month[5]);} // JxN or JxN to JUN
  }
// special case for JUN and JUL
if (strcmp(monthStrOut, month[5]) == 0){ // first month wwhere error would occur
  strcpy(monthStrOut, monthStrIn); // revert if correct month not actually known
  if (monthStrIn[2] == 'N') {strcpy(monthStrOut, month[5]);} // JUx to JUN 
  if (monthStrIn[2] == 'L') {strcpy(monthStrOut, month[6]);} // JUx to JUL
  } 
// special case for MAR and MAY
if (strcmp(monthStrOut, month[2]) == 0){ // first month wwhere error would occur
  strcpy(monthStrOut, monthStrIn); // revert if correct month not actually known
  if (monthStrIn[2] == 'R') {strcpy(monthStrOut, month[2]);} // MAx to MAR 
  if (monthStrIn[2] == 'Y') {strcpy(monthStrOut, month[4]);} // MAx to MAY
  }
displayLine2Out[0] = monthStrOut[0];
displayLine2Out[1] = monthStrOut[1];
displayLine2Out[2] = monthStrOut[2];
return;
}

bool checkChar(char c)
{
char validChar[74] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#*<>()/:-_";
// added last two - and _ for toggle function
// 62 plus 9 plus \0 = 72
int number = strlen(validChar);
for (int i = 0; i < number; i++){ // check c against valid chars
  if (c == validChar[i]) {return true;}  // valid character found
  }
  return false;
}

