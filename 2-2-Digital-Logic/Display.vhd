Library IEEE;
Use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

Entity Display Is
	Port (
		counting_i: In std_logic_vector (23 downto 0);
		blink_i: In std_logic;
		out0_o: Out std_logic_vector (6 downto 0);
		out1_o: Out std_logic_vector (3 downto 0);
		out2_o: Out std_logic_vector (3 downto 0);
		out3_o: Out std_logic_vector (3 downto 0);
		out4_o: Out std_logic_vector (3 downto 0);
		out5_o: Out std_logic_vector (3 downto 0)
	);
End Entity;

Architecture fDisplay Of Display Is
	Signal tmp: std_logic_vector (23 downto 0);
Begin
	Process (blink_i, counting_i, tmp)
	Begin
--		Lock Hell
--		If lock_i'Event And lock_i ='1' Then
			tmp <= counting_i;
--		End If;

		If blink_i = '1' Then
			out5_o <= tmp (23 downto 20);
			out4_o <= tmp (19 downto 16);
			out3_o <= tmp (15 downto 12);
			out2_o <= tmp (11 downto 8);
			out1_o <= tmp (7 downto 4);

			Case tmp (3 downto 0) Is  
				When "0000" => out0_o <= "1111110"; 
				When "0001" => out0_o <= "0110000"; 
				When "0010" => out0_o <= "1101101"; 
				When "0011" => out0_o <= "1111001"; 
				When "0100" => out0_o <= "0110011"; 
				When "0101" => out0_o <= "1011011"; 
				When "0110" => out0_o <= "0011111"; 
				When "0111" => out0_o <= "1110000"; 
				When "1000" => out0_o <= "1111111"; 
				When "1001" => out0_o <= "1111011"; 
				When Others => out0_o <= "0000000";
			End Case;
		Else
			out5_o <= "1111";
			out4_o <= "1111";
			out3_o <= "1111";
			out2_o <= "1111";
			out1_o <= "1111";
			out0_o <= "0000000";
		End If;
	End Process;
End Architecture;