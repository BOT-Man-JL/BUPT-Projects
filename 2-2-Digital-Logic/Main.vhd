Library IEEE;
Use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

Entity Main Is
	Port (
		clock: In std_logic;
		input: In std_logic;
		out0: Out std_logic_vector (6 downto 0);
		out1: Out std_logic_vector (3 downto 0);
		out2: Out std_logic_vector (3 downto 0);
		out3: Out std_logic_vector (3 downto 0);
		out4: Out std_logic_vector (3 downto 0);
		out5: Out std_logic_vector (3 downto 0)
	);
End Entity;

Architecture fMain Of Main Is

Component Control
	Port (
		clk_i: In std_logic;
		clr_o: Out std_logic;
		enable_o: Out std_logic;
		blink_o: Out std_logic
	);
End Component;

Component Counter
	Port (
		clk_i: In std_logic;
		clr_i: In std_logic;
		enable_i: In std_logic;
		counting_o: Out std_logic_vector (23 downto 0)
	);
End Component;

Component Display
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
End Component;

Signal g_clr: std_logic;
Signal g_enable: std_logic;
Signal g_blink: std_logic;
Signal g_counting: std_logic_vector (23 downto 0);

Begin

c1: Control Port Map
(
	clk_i => clock,
	clr_o => g_clr,
	enable_o => g_enable,
	blink_o => g_blink
);

c2: Counter Port Map
(
	clk_i => input,
	clr_i => g_clr,
	enable_i => g_enable,
	counting_o => g_counting
);

c3: Display Port Map
(
	counting_i => g_counting,
	blink_i => g_blink,
	out0_o => out0,
	out1_o => out1,
	out2_o => out2,
	out3_o => out3,
	out4_o => out4,
	out5_o => out5
);

End Architecture;