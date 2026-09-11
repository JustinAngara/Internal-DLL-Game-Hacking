
# IDEA BOARD
- Create something to track time
- to track game tick sent
- embedded message to deobfuscate payload correctly
- to deobfuscate/obfuscate, we account for the game tick sent and should be within a reasonable buffer for a sent time
- make it HWID dependent to prevent distribution

# Schema, expectations, etc

# Call from client to server

{

	// instructions in where they lie
	int    offsetToPlayerUUID
	int    offsetToDetectionFlag // request a mem dump, detected driver, untrue memory, etc
	int    offsetToHWID
	BYTE[] offsetToMemIntegrityCheck
	.... there could be more flags, but keep this structure
	...  there should be heavy obfuscation after this, add padding and footing
	// now this is the real stuff
	UINT64 PlayerUUID
	int DetectionFlag
	int HWID

}




# Call from server to client

# ack needs to be recieved and heavily obfuscated, utilize factors like 

# expected time to live, HWID should match from once called, some sort of memory match 

# important, if reinterpret cast fails when reading payload from server to client by not recieving an ack

# create a request for a memory dump

{

	// instructions in where they lie
	int offsetToPlayerUUID
	int offsetToDetectionFlag
	int offsetToHWID
	.... there could be more flags, but keep this structure
	// now this is the real stuff
	UINT64 PlayerUUID
	int DetectionFlag
	int HWID

}