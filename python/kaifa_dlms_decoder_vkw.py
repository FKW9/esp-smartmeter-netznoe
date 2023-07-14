import datetime

from Crypto.Cipher import AES

# # REAL DATA FROM VKW
KEY = bytes.fromhex('XXXXXXXXXXXXXXXXXXXXX')
DATA = bytes.fromhex('68FAFA6853FF000167DB084B464D1020043BD78201552100008FA2DF4E42850F1A76EB5A935E4236EA7D26107644C7C163229F2538BEF9E354016C5DFC2DAC071DD083FDD2AB1EB4F335F33FBCE54A88B5C97A529E979B614290FDB2B5DF760A4E88D6984EF23D79C48336D16F299FC3BF16C8072FCCA23EE667C8031757DBFC1FF24EE6738465C90E675D90611BA740CE9125A0300853C0F0D95828E043B50184AAA43FF8335BCE02D6E8A7B5F7818923FBFE30AAC3F619F8B4AF778DAC38BFB3CCFFC59C2231E90E067C5911DC935320E655C2CF1C04CA3805EBED05163904138F2BE3C7FDE84F7FCEAC9519F99D3343BD64FCB110BAD698BF67AE112EB4166872726853FF110167355183CB78C9E3C40670CC906A45E6133862A3847E453A5BC811D3803D98C1C47E161FB0F1C7478E65AB29A51F790AC0AF685B136E37B4A9E7FA10C27DB29E4E29B84A8603120FEB4DFC4EF587F6B3E8148E4570FA99BA97B3C941A097A79D3558102C249ACBEF12561AA58EF2E416')

HEADER_POS_1 = 27
HEADER_POS_2 = 9

OBIS_LENGTH = 12

H_SEP = '\n' + '=' * 80

if __name__ == '__main__':

    msglen1 = int(hex(DATA[1]), 16)
    print(msglen1)
    # msglen1 = 228

    splitinfo = DATA[6]         # wenn hier 00, dann gibts noch eine Nachricht
    systitle  = DATA[11:19]     # hier steht der SysTitle
    print('System Title: ' + systitle.hex())
    ic = DATA[23:27]            # hier steht der invectation counter
    iv = systitle + ic          # iv ist 12 Bytes
    print('Initialisierungsvektor: ' + iv.hex() + '\nLänge:', len(iv), H_SEP)

    msg1 = DATA[HEADER_POS_1 : (6 + msglen1-2)]
    # msg1 = DATA[26:msglen1]
    print('Frame 1 ['+ str(len(msg1)) + ']: ' + msg1.hex() + H_SEP)

    msglen2 = int(hex(DATA[msglen1 + 7]), 16)
    print(msglen2)
    # msglen2 = 243 - 228

    msg2 = DATA[msglen1 + 6 + HEADER_POS_2 : (msglen1 + 5 + 5 + msglen2)]
    # msg2 = DATA[256+9:msglen2]
    print('Frame 2 ['+ str(len(msg2)) + ']: ' + msg2.hex() + H_SEP)

    cyphertext = msg1  +  msg2
    print('Cyphertext:\n' + cyphertext.hex() + H_SEP)

    cyphertext_bytes = bytes.fromhex(cyphertext.hex())
    cipher           = AES.new(KEY, AES.MODE_GCM, nonce=iv)
    decrypted        = cipher.decrypt(cyphertext_bytes)

    # OBIS Code Werte aus decrypted.hex auslesen
    databin = decrypted.hex()

    print('Encoded DATA:\n' + databin + H_SEP)

    obis_zeitstempel_pos = databin.find('0000010000ff')

    if obis_zeitstempel_pos != -1:
        obis_datum_zeit = databin[obis_zeitstempel_pos + OBIS_LENGTH + 4:obis_zeitstempel_pos + OBIS_LENGTH + 24]

        print(obis_datum_zeit)
        print('07E7070D04110B1E00FF8880')

        jahr  = int(obis_datum_zeit[:4], 16)
        monat = int(obis_datum_zeit[4:6], 16)
        tag   = int(obis_datum_zeit[6:8], 16)
        stunde  = int(obis_datum_zeit[10:12], 16)
        minute  = int(obis_datum_zeit[12:14], 16)
        sekunde = int(obis_datum_zeit[14:16], 16)

        obis_datum_zeit = datetime.datetime(jahr, monat, tag, stunde, minute, sekunde)
        print('0.0.1.0.0.255\tDatum Zeit:\t\t\t ' + obis_datum_zeit.strftime('%d.%m.%Y %H:%M:%S'))

    else:
        print('\n*** kann OBIS Code für Zeitstempel nicht finden  ***\n')


    # Zählernummer des Netzbetreibers
    # 0000600100ff
    obis_zaehlernummer = '0000600100ff'
    obis_zaehlernummer_pos = databin.find(obis_zaehlernummer)
    if obis_zaehlernummer_pos > 1:

        obis_zaehlernummer_anzzeichen = 2*int(databin[obis_zaehlernummer_pos + OBIS_LENGTH + 2 : obis_zaehlernummer_pos + OBIS_LENGTH + 4], 16)

        obis_zaehlernummer = databin[obis_zaehlernummer_pos + OBIS_LENGTH + 4 : obis_zaehlernummer_pos + OBIS_LENGTH + 4 + obis_zaehlernummer_anzzeichen]
        bytes_object = bytes.fromhex(obis_zaehlernummer)
        print('0.0.96.1.0.255\tZaehlernummer:\t\t\t ' + bytes_object.decode('ASCII'))


    # COSEM Logical Device Name
    # 00002a0000ff
    obis_cosemlogdevname = '00002a0000ff'
    obis_cosemlogdevname_pos = databin.find(obis_cosemlogdevname)
    if obis_cosemlogdevname_pos > 1:

        obis_cosemlogdevname_anzzeichen = 2*int(databin[obis_cosemlogdevname_pos + OBIS_LENGTH + 2 : obis_cosemlogdevname_pos + OBIS_LENGTH + 4], 16)

        obis_cosemlogdevname = databin[obis_cosemlogdevname_pos + OBIS_LENGTH + 4 : obis_cosemlogdevname_pos + OBIS_LENGTH + 4 + obis_cosemlogdevname_anzzeichen]
        bytes_object = bytes.fromhex(obis_cosemlogdevname)
        print('0.0.42.0.0.255\tCOSEM logical device name:\t ' + bytes_object.decode('ASCII'))


    # Spannung L1 (V)
    # 0100200700ff
    obis_spannungl1 = '0100200700ff'
    obis_spannungl1_pos = databin.find(obis_spannungl1)
    if obis_spannungl1_pos > 1:

        obis_spannungl1_anzzeichen = 4
        obis_spannungl1 = databin[obis_spannungl1_pos + OBIS_LENGTH + 2 : obis_spannungl1_pos + OBIS_LENGTH + 2 + obis_spannungl1_anzzeichen]
        print('1.0.32.7.0.255\tSpannung L1 (V):\t\t ' + str(int(obis_spannungl1, 16) / 10))


    # Spannung L2 (V)
    # 0100340700FF
    obis_spannungl2 = '0100340700ff'
    obis_spannungl2_pos = databin.find(obis_spannungl2)
    if obis_spannungl2_pos > 1:

        obis_spannungl2_anzzeichen = 4
        obis_spannungl2 = databin[obis_spannungl2_pos + OBIS_LENGTH + 2 : obis_spannungl2_pos + OBIS_LENGTH + 2 + obis_spannungl2_anzzeichen]
        print('1.0.52.7.0.255\tSpannung L2 (V):\t\t ' + str(int(obis_spannungl2, 16) / 10))


    # Spannung L3 (V)
    # 0100480700ff
    obis_spannungl3 = '0100480700ff'
    obis_spannungl3_pos = databin.find(obis_spannungl3)
    if obis_spannungl3_pos > 1:

        obis_spannungl3_anzzeichen = 4
        obis_spannungl3 = databin[obis_spannungl3_pos + OBIS_LENGTH + 2 : obis_spannungl3_pos + OBIS_LENGTH + 2 + obis_spannungl3_anzzeichen]
        print('1.0.72.7.0.255\tSpannung L3 (V):\t\t ' + str(int(obis_spannungl3, 16) / 10))


    # Strom L1 (A)
    # 01001f0700ff
    obis_stroml1 = '01001f0700ff'
    obis_stroml1_pos = databin.find(obis_stroml1)
    if obis_stroml1_pos > 1:

        obis_stroml1_anzzeichen = 4
        obis_stroml1 = databin[obis_stroml1_pos + OBIS_LENGTH + 2 : obis_stroml1_pos + OBIS_LENGTH + 2 + obis_stroml1_anzzeichen]
        print(databin[obis_stroml1_pos:obis_stroml1_pos+20])
        print('1.0.31.7.0.255\tStrom L1 (A):\t\t\t ' + str(int(obis_stroml1, 16) / 100))


    # Strom L2 (A)
    # 0100330700ff
    obis_stroml2 = '0100330700ff'
    obis_stroml2_pos = databin.find(obis_stroml2)
    if obis_stroml2_pos > 1:

        obis_stroml2_anzzeichen = 4
        obis_stroml2 = databin[obis_stroml2_pos + OBIS_LENGTH + 2 : obis_stroml2_pos + OBIS_LENGTH + 2 + obis_stroml2_anzzeichen]
        print('1.0.51.7.0.255\tStrom L2 (A):\t\t\t ' + str(int(obis_stroml2, 16) / 100))


    # Strom L3 (A)
    # 0100470700ff
    obis_stroml3 = '0100470700ff'
    obis_stroml3_pos = databin.find(obis_stroml3)
    if obis_stroml3_pos > 1:

        obis_stroml3_anzzeichen = 4
        obis_stroml3 = databin[obis_stroml3_pos + OBIS_LENGTH + 2 : obis_stroml3_pos + OBIS_LENGTH + 2 + obis_stroml3_anzzeichen]
        print('1.0.71.7.0.255\tStrom L3 (A):\t\t\t ' + str(int(obis_stroml3, 16) / 100))


    # Wirkleistung Bezug  + P (W)
    # 0100010700ff
    obis_wirkleistungbezug = '0100010700ff'
    obis_wirkleistungbezug_pos = databin.find(obis_wirkleistungbezug)
    if obis_wirkleistungbezug_pos > 1:

        obis_wirkleistungbezug_anzzeichen = 8

        obis_wirkleistungbezug = databin[obis_wirkleistungbezug_pos + OBIS_LENGTH + 2 : obis_wirkleistungbezug_pos + OBIS_LENGTH + 2 + obis_wirkleistungbezug_anzzeichen]
        print('1.0.1.7.0.255\tWirkleistung Bezug [kW]:\t ' + str(int(obis_wirkleistungbezug, 16) / 1000))


    # Wirkleistung Lieferung -P (W)
    # 0100020700ff
    obis_wirkleistunglieferung = '0100020700ff'
    obis_wirkleistunglieferung_pos = databin.find(obis_wirkleistunglieferung)
    if obis_wirkleistunglieferung_pos > 1:

        obis_wirkleistunglieferung_anzzeichen = 8

        obis_wirkleistunglieferung = databin[obis_wirkleistunglieferung_pos + OBIS_LENGTH + 2 : obis_wirkleistunglieferung_pos + OBIS_LENGTH + 2 + obis_wirkleistunglieferung_anzzeichen]
        print('1.0.2.7.0.255\tWirkleistung Lieferung [kW]:\t ' + str(int(obis_wirkleistunglieferung, 16) / 1000))


    # Wirkenergie Bezug  + A (Wh)
    # 0100010800ff
    obis_wirkenergiebezug = '0100010800ff'
    obis_wirkenergiebezug_pos = databin.find(obis_wirkenergiebezug)
    if obis_wirkenergiebezug_pos > 1:

        obis_wirkenergiebezug_anzzeichen = 8

        obis_wirkenergiebezug = databin[obis_wirkenergiebezug_pos + OBIS_LENGTH + 2 : obis_wirkenergiebezug_pos + OBIS_LENGTH + 2 + obis_wirkenergiebezug_anzzeichen]
        print(obis_wirkenergiebezug)
        print('1.0.1.8.0.255\tWirkenergie Bezug [kWh]:\t ' + str(int(obis_wirkenergiebezug, 16) / 1000))


    # Wirkenergie Lieferung -A (Wh)
    # 0100020800ff
    obis_wirkenergielieferung = '0100020800ff'
    obis_wirkenergielieferung_pos = databin.find(obis_wirkenergielieferung)
    if obis_wirkenergielieferung_pos > 1:

        obis_wirkenergielieferung_anzzeichen = 8

        obis_wirkenergielieferung = databin[obis_wirkenergielieferung_pos + OBIS_LENGTH + 2 : obis_wirkenergielieferung_pos + OBIS_LENGTH + 2 + obis_wirkenergielieferung_anzzeichen]
        print('1.0.2.8.0.255\tWirkenergie Lieferung [kWh]:\t ' + str(int(obis_wirkenergielieferung, 16) / 1000))


    # Blindleistung Bezug  + R (Wh)
    # 0100030800ff
    obis_blindleistungbezug = '0100030800ff'
    obis_blindleistungbezug_pos = databin.find(obis_blindleistungbezug)
    if obis_blindleistungbezug_pos > 1:

        obis_blindleistungbezug_anzzeichen = 8

        obis_blindleistungbezug = databin[obis_blindleistungbezug_pos + OBIS_LENGTH + 2 : obis_blindleistungbezug_pos + OBIS_LENGTH + 2 + obis_blindleistungbezug_anzzeichen]
        print('1.0.3.8.0.255\tBlindleistung Bezug [kW]:\t ' + str(int(obis_blindleistungbezug, 16) / 1000))


    # Blindleistung Lieferung -R (Wh)
    # 0100040800ff
    obis_blindleistunglieferung = '0100040800ff'
    obis_blindleistunglieferung_pos = databin.find(obis_blindleistunglieferung)
    if obis_blindleistunglieferung_pos > 1:

        obis_blindleistunglieferung_anzzeichen = 8

        obis_blindleistunglieferung = databin[obis_blindleistunglieferung_pos + OBIS_LENGTH + 2 : obis_blindleistunglieferung_pos + OBIS_LENGTH + 2 + obis_blindleistunglieferung_anzzeichen]
        print('1.0.4.8.0.255\tBlindleistung Lieferung [kW]:\t ' + str(int(obis_blindleistunglieferung, 16) / 1000))


    # Leistungsfaktor
    # 01000D0700FF
    obis_leistungsfaktor = '01000d0700ff'
    obis_leistungsfaktor_pos = databin.find(obis_leistungsfaktor)
    if obis_leistungsfaktor_pos > 1:

        obis_leistungsfaktor_anzzeichen = 4

        obis_leistungsfaktor = databin[obis_leistungsfaktor_pos + OBIS_LENGTH + 2 : obis_leistungsfaktor_pos + OBIS_LENGTH + 2 + obis_leistungsfaktor_anzzeichen]
        print('1.0.13.7.0.255\tLeistungsfaktor [cos(phi)]:\t ' + str(int(obis_leistungsfaktor, 16) / 1000))

