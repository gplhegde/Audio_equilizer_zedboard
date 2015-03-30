library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-------------------------------------------------------------------------------------
--
--
-- Definition of Ports
-- ACLK              : Synchronous clock
-- ARESETN           : System reset, active low
-- S_AXIS_TREADY  : Ready to accept data in
-- S_AXIS_TDATA   :  Data in 
-- S_AXIS_TLAST   : Optional data in qualifier
-- S_AXIS_TVALID  : Data in is valid
-- M_AXIS_TVALID  :  Data out is valid
-- M_AXIS_TDATA   : Data Out
-- M_AXIS_TLAST   : Optional data out qualifier
-- M_AXIS_TREADY  : Connected slave device is ready to accept data out
--
-------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Entity Section
------------------------------------------------------------------------------

entity fft_128_v8_stream is
	port 
	(
		-- DO NOT EDIT BELOW THIS LINE ---------------------
		-- Bus protocol ports, do not add or delete. 
		ACLK	: in	std_logic;
		ARESETN	: in	std_logic;
		S_AXIS_TREADY	: out	std_logic;
		S_AXIS_TDATA	: in	std_logic_vector(31 downto 0);
		S_AXIS_TLAST	: in	std_logic;
		S_AXIS_TVALID	: in	std_logic;
		M_AXIS_TVALID	: out	std_logic;
		M_AXIS_TDATA	: out	std_logic_vector(31 downto 0);
		M_AXIS_TLAST	: out	std_logic;
		M_AXIS_TREADY	: in	std_logic
		-- DO NOT EDIT ABOVE THIS LINE ---------------------
	);

attribute SIGIS : string; 
attribute SIGIS of ACLK : signal is "Clk"; 

end fft_128_v8_stream;

------------------------------------------------------------------------------
-- Architecture Section
------------------------------------------------------------------------------


architecture EXAMPLE of fft_128_v8_stream is

	signal s_axis_config_tdata: STD_LOGIC_VECTOR(15 DOWNTO 0);
	signal s_axis_config_tvalid: STD_LOGIC;
	signal s_axis_config_tready: STD_LOGIC;
	signal event_frame_started: STD_LOGIC;
	signal event_tlast_unexpected: STD_LOGIC;
	signal event_tlast_missing:STD_LOGIC;
	signal event_status_channel_halt:STD_LOGIC;
	signal event_data_in_channel_halt:STD_LOGIC;
	signal event_data_out_channel_halt:STD_LOGIC;
	
	
	
COMPONENT fft_stream_128
  PORT (
    aclk : IN STD_LOGIC;
    s_axis_config_tdata : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    s_axis_config_tvalid : IN STD_LOGIC;
    s_axis_config_tready : OUT STD_LOGIC;
    s_axis_data_tdata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    s_axis_data_tvalid : IN STD_LOGIC;
    s_axis_data_tready : OUT STD_LOGIC;
    s_axis_data_tlast : IN STD_LOGIC;
    m_axis_data_tdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    m_axis_data_tvalid : OUT STD_LOGIC;
    m_axis_data_tready : IN STD_LOGIC;
    m_axis_data_tlast : OUT STD_LOGIC;
    event_frame_started : OUT STD_LOGIC;
    event_tlast_unexpected : OUT STD_LOGIC;
    event_tlast_missing : OUT STD_LOGIC;
    event_status_channel_halt : OUT STD_LOGIC;
    event_data_in_channel_halt : OUT STD_LOGIC;
    event_data_out_channel_halt : OUT STD_LOGIC
  );
END COMPONENT;
   

begin
   
	fft_stream_128_inst : fft_stream_128
  PORT MAP (
    aclk => ACLK,
    s_axis_config_tdata => s_axis_config_tdata,
    s_axis_config_tvalid => s_axis_config_tvalid,
    s_axis_config_tready => s_axis_config_tready,
    s_axis_data_tdata => S_AXIS_TDATA,
    s_axis_data_tvalid => S_AXIS_TVALID,
    s_axis_data_tready => S_AXIS_TREADY,
    s_axis_data_tlast => S_AXIS_TLAST,
    m_axis_data_tdata => M_AXIS_TDATA,
    m_axis_data_tvalid => M_AXIS_TVALID,
    m_axis_data_tready => M_AXIS_TREADY,
    m_axis_data_tlast => M_AXIS_TLAST,
    event_frame_started => event_frame_started,
    event_tlast_unexpected => event_tlast_unexpected,
    event_tlast_missing => event_tlast_missing,
    event_status_channel_halt => event_status_channel_halt,
    event_data_in_channel_halt => event_data_in_channel_halt,
    event_data_out_channel_halt => event_data_out_channel_halt
  );
   
end architecture EXAMPLE;
