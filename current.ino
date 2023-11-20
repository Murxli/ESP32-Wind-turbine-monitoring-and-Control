void setup()
{
  Serial.begin(9600);

}
void loop()
{
  float average=0;
  for(int i=0;i<1000;i++)
  {
    average=average+(0.19*analogRead(A0)-25)/1000;
    delay(1);
  }
  Serial.print("Current: ");
  Serial.print(average);
  Serial.println("A");
