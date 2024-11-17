-- Prgramme lecture heur/minute pour envoi vers chrono
-- Declaration variable 238/239 Heure locale 23B/23C ZOULOU
Heure_0=0
Minute_0=0
seconde_0=0
Heure_1=0
Minute_1=0
Seconde_1=0
Sys_Heure_1 = 0
Sys_Minute_1 = 0
Sys_Seconde_1 = 0
--Systeme=os.time()
--Sys_heure_0=tonumber(os.date('%H',Systeme))
--Sys_minute_0=tonumber(os.date('%M',Systeme))

dog=false
-- ouverture du port
 handle=com.open("COM3",57600,1)
while 1 do
	str,m = com.read(handle,1)

	if(m > 0) then
	    if((str == "A")) then
		dog = false
		print ("reception :",str)
		else
			print(str)
		end
	end

	if((dog == false) and (str=="A"))then
			Heure_1 = ipc.readUB(0x0238)
			ipc.sleep(10)
			Minute_1 = ipc.readUB(0x0239)
			ipc.sleep(10)
			Seconde_1 = ipc.readUB(0x23A)
			ipc.sleep(10)
			--tps= os.time{ hour=hour, mint=minutes, sec=seconds}
			
			heure = os.date ("%H")
			minute = os.date("%M")
			seconde = os.date("%S")
			ipc.sleep(10)
--print (os.date ("%H"))
--print (heure)
			if(m > 0) then
				dog = true
				--print("dog")
				str=Heure_1..";"..Minute_1..";"..Seconde_1..";"..heure..";"..minute..";"..seconde.."\n"
				n=com.write(handle,str)
				--print( " Transmis init : ",  str)
				Minute_0=Minute_1;
			end
	else
		Heure_1 = ipc.readUB(0x0238)
		ipc.sleep(10)
		Minute_1 = ipc.readUB(0x0239)
		ipc.sleep(10)
		Seconde_1 = ipc.readUB(0x23A)
		ipc.sleep(10)
			--tps= os.date{ hour1=%H, mint1=%M, sec1=%S}
		heure1 = os.date ("%H")
			minute1 = os.date("%M")
			seconde1 = os.date("%S")
		ipc.sleep(10)
		if Minute_1 ~= Minute_0 then
			str=Heure_1..";"..Minute_1..";"..Seconde_1..";"..heure1..";"..minute1..";"..seconde1.."\n"
			n=com.write(handle,str)
			--print(" nombre caractères envoyés : ",n)
			--print("Mise Ã  jour heure : ", str);
			Minute_0=Minute_1;
			ipc.sleep(500);
		end
	end
end


