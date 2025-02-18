#!/usr/bin/python
# -*- coding:utf-8 -*-
import RPi.GPIO as GPIO
import time


def send_at(ser,rec_buff,command,back,timeout):
	rec_buff = ''
	ser.write((command+'\r\n').encode())
	time.sleep(timeout)
	if ser.in_waiting():
		time.sleep(0.01)
		rec_buff = ser.read(ser.in_waiting())
	if rec_buff != '':
		if back not in rec_buff.decode():
			print(command + ' ERROR')
			print(command + ' back:\t' + rec_buff.decode())
			return 0
		else:
			print(rec_buff.decode())
			return 1
	else:
		print('GPS is not ready')
		return 0

def get_gps_position(ser):
	answer = 0
	print('Start GPS session...')
	rec_buff = ''
	send_at(ser,rec_buff,'AT+CGPS=1,1','OK',1)
	time.sleep(2)
	answer = send_at(ser,rec_buff,'AT+CGPSINFO','+CGPSINFO: ',1)
	if 1 == answer:
		if ',,,,,,' in rec_buff:
			return 'GPS cannot get position'
		else:
			if rec_buff.startswith("+CGPSINFO: "):
				return rec_buff[11:]
			return rec_buff
	else:
		print('error %d'%answer)
		rec_buff = ''
		send_at(ser,rec_buff,'AT+CGPS=0','OK',1)
		return 'GPS Unavailable'


def power_on(ser,power_key):
	print('SIM7600X is starting:')
	GPIO.setmode(GPIO.BCM)
	GPIO.setwarnings(False)
	GPIO.setup(power_key,GPIO.OUT)
	time.sleep(0.1)
	GPIO.output(power_key,GPIO.HIGH)
	time.sleep(2)
	GPIO.output(power_key,GPIO.LOW)
	time.sleep(20)
	ser.reset_input_buffer()
	print('SIM7600X is ready')

def power_down(power_key):
	print('SIM7600X is loging off:')
	GPIO.output(power_key,GPIO.HIGH)
	time.sleep(3)
	GPIO.output(power_key,GPIO.LOW)
	time.sleep(18)
	GPIO.cleanup()
	print('Good bye')
