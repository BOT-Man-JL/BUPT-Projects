Library IEEE;
Use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

Entity Control Is
	Port (
		clk_i: In std_logic;
		clr_o: Out std_logic;
		enable_o: Out std_logic;
		blink_o: Out std_logic
	);
End Entity;

Architecture fControl Of Control Is
	Signal g_clk: std_logic;
	Signal clr_t: std_logic;
	Signal enable_t: std_logic;
	Signal blink_t: std_logic;
Begin
	Process (clk_i)
		Variable tmp: Integer Range 0 to 1000;
	Begin
		If clk_i'Event And clk_i = '1' Then
			If tmp = 999 Then
				tmp := 0;
				g_clk <= '1';
			Else
				tmp := tmp + 1;
				g_clk <= '0';
			End If;
		End If;
	End Process;

	Process (g_clk)
		Variable count: Integer Range 0 to 1000;
	Begin
		If g_clk'Event And g_clk = '1' Then
			If count = 999 Then
				count := 0;
				enable_t <= Not enable_t;
				clr_t <= '0';
			Elsif enable_t = '1' Then
				count := count + 1;
				clr_t <= '0';
				blink_t <= '1';
			Elsif count > 499 Then
				count := count + 1;
				clr_t <= '1';
				blink_t <= '0';
			Else
				count := count + 1;
				blink_t <= '1';
			End If;
		End If;
	End Process;

	clr_o <= clr_t;
	enable_o <= enable_t;
	blink_o <= blink_t;
End Architecture;