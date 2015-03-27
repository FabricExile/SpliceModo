//
#ifndef SERVER_NAME_cmdLogFabricVersion
#define	SERVER_NAME_cmdLogFabricVersion "LogFabricVersion"

namespace cmdLogFabricVersion
{
	class Command : public CLxBasicCommand
	{
		public:

		// tag description interface.
		static LXtTagInfoDesc descInfo[];

		// initialization.
		static void initialize(void)
		{
			CLxGenericPolymorph *srv =   new CLxPolymorph			<Command>;
			srv->AddInterface			(new CLxIfc_Command			<Command>);
			srv->AddInterface			(new CLxIfc_StaticDesc		<Command>);
			lx:: AddServer				(SERVER_NAME_cmdLogFabricVersion, srv);
		};

		// command service.
		int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
		bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
		void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
	};
};

#endif

