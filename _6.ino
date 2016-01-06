#include <Time.h>
#include <TimeLib.h>

#define limit 280
#define order_gap_delay 5000
int check_lux();
void auto_pilot();
void admin();
void parse_answer(char received[] , char expect[],int );
int cnt,led,str_length,i,wait_time,top,top_limit,lux,last_lux,sum,lux_cnt,lux_delay,lux_gap;
bool auto_admin ,data_rdy,r_flag;
char info[limit]={'\0',};
char order_log[10][limit]={{"at+wauto=0,SeaN"},
                            {"at+wwpa=34265013"},
                             {"at+ndhcp=1"},
                             {"at+wstart"},
                             {"at+nauto=0,1,223.194.115.119,8080"},
                             {"at+socket"},
                             {""},
                             {""},
                             {"at+reset"}},
      return_log[10][limit]={{"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[WIFI CONNECT]|[WIFI DISCONNECT]|"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"},
                             {"[OK]|[ERROR]|Serial2WiFi APP"}};

void setup() {
  // put your setup code here, to run once:
  cnt = 0;
  led = 13;
  wait_time = 1000;
  top = 0;
  top_limit = 9;
  last_lux = -1;
  auto_admin = true;
  data_rdy = false;
  r_flag = true;
  sum = 0;
  lux_cnt = 0;
  lux_delay = 1000;
  lux_gap = 30;
  
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);//led 성가심

  sprintf(info,"buffer limit size : %d\nlimit wait time : %dms\nauto pilot : %d\nlux gap : %d lux delay : %dms\ninit ok made by stories2",limit,wait_time*10,auto_admin,lux_gap,lux_delay);
  Serial.println(info);
  delay(100);

  setTime(18,49,00,6,1,2016);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(data_rdy == false)
  {
    if(r_flag == true)
    {
      sum = 0;
      lux_cnt = 0;
      r_flag = false;
    }
    lux_cnt += 1;
    sum = sum + check_lux();
    digitalWrite(led,LOW);
    delay(lux_delay);
    digitalWrite(led,HIGH);
    if(lux_cnt>=10)
    {
      char print_buffer[limit] = {'\0',};
      if(last_lux == -1)
      {
        last_lux = sum/10;
        r_flag = true;
        sprintf(print_buffer,"last lux set %d",last_lux);
        Serial.println(print_buffer);
      }
      else
      {
        if(abs(last_lux - sum/10)>lux_gap)
        {
          sprintf(print_buffer,"data rdy %d : %d",last_lux,sum/10);
          Serial.println(print_buffer);
          lux = sum/10;
          last_lux = sum/10;
          data_rdy = true;
        }
        else
        {
          sprintf(print_buffer,"data reset %d : %d",last_lux,sum/10);
          Serial.println(print_buffer);
          r_flag = true;
        }
      }
    }
  }
  else
  {
    r_flag = true;
    char _buffer[limit] = {'\0',};
  if(auto_admin == false)
  {
    admin();
  }
  else
  {
    auto_pilot();
  }
  Serial1.write(13);
  Serial1.flush();
  cnt = 0; 
  while(!Serial1.available())
  {
    if(cnt == 0)
    {
      Serial.print("\nwifi > ");
    }
    cnt+=1;
    if(cnt%50 == 0)
    {
      Serial.println();
    }
    delay(10);
    Serial.print(".");
    if(cnt>wait_time)
    {
      Serial.println("time out");
      break;
    }
  }
  cnt=0;
  while(Serial1.available())
  {
    _buffer[cnt] = Serial1.read();
    Serial.print(_buffer[cnt]);
    digitalWrite(led,HIGH);
    delay(10);
    digitalWrite(led,LOW);
    cnt+=1;
    if(cnt>limit)
    {
      Serial.println("buffer overflow");
      break;
    }
  }
  if(auto_admin == true)
  {
    parse_answer(_buffer,return_log[top%top_limit],top%top_limit);
  }
  Serial.println(); 
  }  
}

void admin()
{
  char _buffer[limit] = {'\0',};
  while(!Serial.available())
  {
    cnt = 0;
  }
  while(Serial.available())
  {
    if(cnt == 0)
    {
      Serial.print("root > ");
    }
    _buffer[cnt] = Serial.read();
    Serial.print(_buffer[cnt]);
    Serial1.write(_buffer[cnt]);
    delay(1);
    cnt+=1;
    if(cnt>limit)
    {
      Serial.println("buffer overflow");
      break;
    }
  }
}

void auto_pilot()
{
  int now_position = top%top_limit;
  cnt = strlen(order_log[now_position]);
  char print_buffer[limit] = {'\0',};
  sprintf(print_buffer,"autopilot #%d -top %d > ",now_position,top);
  Serial.print(print_buffer);

  if(now_position == 0)
  {
    
  }
  if(now_position == 6)
  {
    sprintf(order_log[now_position],"at+send=update product_light set LUX=%d where ProductMac = 'LK:JI:HG:FE:DC:BA';",lux);
  }
  if(now_position == 7)
  {
    sprintf(order_log[now_position],"at+send=insert into product_log value('%d-%d-%d %d:%d:%d','LK:JI:HG:FE:DC:BA',0,1,'','',%d);",year(),month(),day(),hour(),minute(),second(),lux);
    
  }
  
  for(i=0;i<cnt;i+=1)
  {
    Serial.print(order_log[now_position][i]);
    Serial1.write(order_log[now_position][i]);
  }
}

void parse_answer(char receive_msg[] , char compare_expect_msg[] , int now_position)
{
  char option[3][limit] = {'\0',};
  int t=0,q=0,answer=0;
  bool check;
  str_length = strlen(compare_expect_msg);
  for(i=0;i<str_length;i+=1)
  {
    if(compare_expect_msg[i] != '|')
    {
      option[t][q] = compare_expect_msg[i];
      q+=1;
    }
    else
    {
      t+=1;
      q=0;
    }
  }
  str_length = cnt;
  for(i=0;i<3;i+=1)
  {
    check = true;
    q=0;
    for(t=0;t<str_length;t+=1)
    {
      if(strlen(option[i])-1<=q)
      {
        break;
      }
      if(option[i][q] == receive_msg[t])
      {
        q+=1;
      }
      else if(q>0 && option[i][q] != receive_msg[t])
      {
        check = false;
        break;
      }
    }
    if(check == true)
    {
      break;
    }
  }
  switch(i)
  {
    case 0:
      if(now_position == top_limit-1)
      {
        Serial.println("arduino > reset delay");
        delay(5000);
      }
      else
      {
        Serial.println("arduino > processing delay");
        if(now_position == 7)
        {
          data_rdy = false;
        }
        delay(order_gap_delay);
      }
      top = top + 1;
      break;
    case 1:
        Serial.println("arduino > resetting");
      top = top_limit-1;
      break;
    case 2:
      Serial.println("arduino > waiting respond");
      delay(5000);
      break;
  }
}

int check_lux()
{
  delay(10);
  return analogRead(A0);
}

