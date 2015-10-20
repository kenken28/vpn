# vpn

This vpn program alows encypted comunication between SERVER and CLIENT.

The prime number used for Diffie-Hellman is from [RFC 3526](https://www.ietf.org/rfc/rfc3526.txt).
> (Referenced from RFC 3526)
> 
> * 2048-bit MODP Group
> 
>    This group is assigned id 14.
> 
>    This prime is: 2^2048 - 2^1984 - 1 + 2^64 * { [2^1918 pi] + 124476 }
> 
>    Its hexadecimal value is:
> 
>       FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1
>       29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD
>       EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245
>       E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED
>       EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D
>       C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F
>       83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D
>       670C354E 4ABC9804 F1746C08 CA18217C 32905E46 2E36CE3B
>       E39E772C 180E8603 9B2783A2 EC07A28F B5C55DF0 6F4C52C9
>       DE2BCBF6 95581718 3995497C EA956AE5 15D22618 98FA0510
>       15728E5A 8AACAA68 FFFFFFFF FFFFFFFF
> 
>    The generator is: 2.

## Terminal version
To use the vpn in Terminal
* 1. Run SERVER: $ python server.py
* 2. Enter the key as prumpted
* 3. Run CLIENT: $ python client.py
* 4. Enter the key as prumpted
* 5. ## Wait for Key Establishment and Mutual Authentication ##
* 6. Enter message to be sent at either SERVER side or CLIENT side.
* 7. Enter "@r" at the opposite side to recieve a message (if n messages have been sent, Enter "@r" for n times to recieve all messages)
* 8. Enter "@e" to exit

## GUI version
To use the vpn in GUI
* ## Working on GUI feature ##
