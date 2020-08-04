# Account:

	- Account: 78929@studmail.htw-aalen.de
	- Password: HSAATIBSPflanze

# User:

	- Account: TimHendrik
	- Password: HSAATIBSPflanze

# Connection

	- Connection String:
		- mongodb+srv://TimHendrik:HSAATIBSPflanze@cluster0-e7c5u.mongodb.net

	- App:
		- mongodb+srv://TimHendrik:HSAATIBSPflanze@cluster0-e7c5u.mongodb.net/TIBS?retryWrites=true&w=majority
  
# DB Schema
	TIBS: DB
		Data: Collection 
			{
				[
					{
					userId:"String",
					groups: [
						{
							groupId:"String",
							devices: 
							[
								{
									deviceId : "String",
									data : 
									{
										pumped : [
											"2020:08:04.13:31",
											"2020:07:04.13:31",
										],
										waterLevel : [
											{
												date: "2020:08:04.13:31",
												value: "20",
											},
											{
												date: "2020:07:04.13:31",
												value: "20",
											}
										],
										moisture : [
											{
												date: "2020:08:04.13:31",
												value: "30",
											},
											{
												date: "2020:07:04.13:31",
												value: "31",
											}
										]
									}
								}
							]
						],
					}
				],
