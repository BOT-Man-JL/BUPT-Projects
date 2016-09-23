Library IEEE;
Use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

Entity Counter Is
	Port (
		clk_i: In std_logic;
		clr_i: In std_logic;
		enable_i: In std_logic;
		counting_o: Out std_logic_vector (23 downto 0)
	);
End Entity;

Architecture fCounter Of Counter Is
	Signal tmp: std_logic_vector (23 downto 0);
	Signal R0, R1, R2, R3, R4, R5: std_logic_vector (3 downto 0);
Begin
	Process (clk_i, clr_i, enable_i, R5 , R4 , R3 , R2 , R1 , R0)
	Begin
		If clr_i = '1' Then
			tmp <= "000000000000000000000000";
			R0 <= "0000";
			R1 <= "0000";
			R2 <= "0000";
			R3 <= "0000";
			R4 <= "0000";
			R5 <= "0000";
		Elsif enable_i = '1' Then
			If clk_i'Event And clk_i = '1' Then
				If R0 = "1001" Then
					R0 <= "0000";
					R1 <= R1 + 1;
					If R1 = "1001" Then
						R1 <= "0000";
						R2 <= R2 + 1;
						If R2 = "1001" Then
							R2 <= "0000";
							R3 <= R3 + 1;
							If R3 = "1001" Then
								R3 <= "0000";
								R4 <= R4 + 1;
								If R4 = "1001" Then
									R4 <= "0000";
									R5 <= R5 + 1;
								Else
									R4 <= R4 + 1;
								End If;
							Else
								R3 <= R3 + 1;
							End If;
						Else
							R2 <= R2 + 1;
						End If;
					Else
						R1 <= R1 + 1;
					End If;
				Else
					R0 <= R0 + 1;
				End If;
			End If;
		End If;

		tmp <= R5 & R4 & R3 & R2 & R1 & R0;
	End Process;

	counting_o <= tmp;
End Architecture;