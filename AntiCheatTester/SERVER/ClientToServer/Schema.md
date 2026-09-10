

# Schema, expectations, etc

# Call to server


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