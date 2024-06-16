import datetime

import serial
from Crypto.Cipher import AES

# EVN EXAMPLE DATA
# KEY = bytes.fromhex('36C66639E48A8CA4D6BC8B282A793BBB')
# DATA = bytes.fromhex('68FAFA6853FF000167DB084B464D675000000981F8200000002388D5AB4F97515AAFC6B88D2F85DAA7A0E3C0C40D004535C397C9D037AB7DBDA329107615444894A1A0DD7E85F02D496CECD3FF46AF5FB3C9229CFE8F3EE4606AB2E1F409F36AAD2E50900A4396FC6C2E083F373233A69616950758BFC7D63A9E9B6E99E21B2CBC2B934772CA51FD4D69830711CAB1F8CFF25F0A329337CBA51904F0CAED88D61968743C8454BA922EB00038182C22FE316D16F2A9F544D6F75D51A4E92A1C4EF8AB19A2B7FEAA32D0726C0ED80229AE6C0F7621A4209251ACE2B2BC66FF0327A653BB686C756BE033C7A281F1D2A7E1FA31C3983E15F8FD16CC5787E6F517166814146853FF110167419A3CFDA44BE438C96F0E38BF83D98316')

# # REAL DATA
KEY = bytes.fromhex('825DC0D167DEEB63F49DAB4F31A86CC7')
DATA = bytes.fromhex('68FAFA6853FF000167DB084B464D675000088181F82000006005C1FA38BCC6D9594E5A7C3659138B7EE99A8347BF50B998F8338581084512A3780CACBF5BCB366744C99E99E53A12E867D4EDE68EF82A91F6D76C086C73AD5754CE8F1313D9F738C850D49362750A9B6FCBE33ADE62A16C62A9B5CAC79331E9786281604BC92422A124E0C3B9C979D0E265DA601403C556B8960944C424BAD49D63ADABFBAEDA075D66882F5CB55A7AAD43FC24470F3B23BD6BBBFF71C6447A3DEB2EA47D8060D7AAF62F149C0DD4CF60FDC91C20ED9A6DCCF06DA54DFB9B5F4560DFB5364E0E01056A07364A605F8575F3841D517D07220614CFC7CF98825EE720C51CBC59166814146853FF1101674CC1D217B8D8034CC7515FE02095610D16')

# KEY = bytes.fromhex('E36344D76C1F6E5DD9F54258B5508866')
# DATA = bytes.fromhex('6801016853FF000167DB085341475905ED331281F8200000157604C19F8D8EAF42B7235E18E38457302F164A91F4788817040C30D9F2DA747BD933D5017B1F3FA2FE95084AB2D2DCBB3CE7572B8FD161D413472673231C04B71588E743803BEFE26675E1226910028C475BBE0F6B348C50452C1F16B6624743D10F4EC824F49254090176FD2D347300B10081B877915C1B70C95E69CFB6AAB2C36FDBC048E41A4AB44E774C7AE3DCD12E33170EA55B13148F6A77B1F083DCAEB04EAEFAF8CCBF9F5D0FAF934D3410219FE642096A0D478BAC97732E73002A09F76819647F2F5FCC45ADE3E9412452D03233A8C72E5F28B004D3B1CF10BA4530C2B5231B774B86F500BD1A15BF16680D0D6853FF1101674033CDD8C8D1DAB50B16')

HEADER_POS_1 = 26
HEADER_POS_2 = 9

OBIS_LENGTH = 12

H_SEP = '\n' + '=' * 80

if __name__ == '__main__':

    msglen1 = int(hex(DATA[1]), 16)
    # msglen1 = 228

    splitinfo = DATA[6]         # wenn hier 00, dann gibts noch eine Nachricht
    systitle  = DATA[11:19]     # hier steht der SysTitle
    print('System Title: ' + systitle.hex())
    ic = DATA[22:26]            # hier steht der invectation counter
    iv = systitle + ic          # iv ist 12 Bytes
    print('Initialisierungsvektor: ' + iv.hex() + '\nLänge:', len(iv), H_SEP)

    msg1 = DATA[HEADER_POS_1 : (6 + msglen1-2)]
    # msg1 = DATA[26:msglen1]
    print('Frame 1 ['+ str(len(msg1)) + ']: ' + msg1.hex() + H_SEP)

    msglen2 = int(hex(DATA[msglen1 + 7]), 16)
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

    obis_zeitstempel_pos = databin.find('0223090c')

    if obis_zeitstempel_pos != -1:
        obis_datum_zeit = databin[44:68]

        print(obis_datum_zeit)

        jahr  = int(obis_datum_zeit[:4], 16)
        monat = int(obis_datum_zeit[4:6], 16)
        tag   = int(obis_datum_zeit[6:8], 16)
        stunde  = int(obis_datum_zeit[10:12], 16)
        minute  = int(obis_datum_zeit[12:14], 16)
        sekunde = int(obis_datum_zeit[14:16], 16)

        obis_datum_zeit = datetime.datetime(jahr, monat, tag, stunde, minute, sekunde)
        print('\t\tDatum Zeit:\t\t\t ' + obis_datum_zeit.strftime('%d.%m.%Y %H:%M:%S'))

    else:
        print('\n*** kann OBIS Code nicht finden  ***\n')


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

