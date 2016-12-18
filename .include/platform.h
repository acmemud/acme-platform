#ifndef _PLATFORM_H
#define _PLATFORM_H

#define PlatformDomain       "platform"

#define PlatformBinDir       PlatformDir _BinDir
#define PlatformEtcDir       PlatformDir _EtcDir
#define PlatformIncludeDir   PlatformDir _IncludeDir
#define PlatformLibDir       PlatformDir _LibDir
#define PlatformModuleDir    PlatformDir _ModuleDir
#define PlatformObjDir       PlatformDir _ObjDir

#define ArgsLib              PlatformLibDir "/args"
#define ArrayLib             PlatformLibDir "/array"
#define ClosureLib           PlatformLibDir "/closure"
#define CommandLib           PlatformLibDir "/command"
#define CommandSpecLib       PlatformLibDir "/command_spec"
#define ConnectionLib        PlatformLibDir "/connection"
#define DomainLib            PlatformLibDir "/domain"
#define FileLib              PlatformLibDir "/file"
#define FormatStringsLib     PlatformLibDir "/format_strings"
#define GetoptsLib           PlatformLibDir "/getopts"
#define JSONLib              PlatformLibDir "/json"
#define MessageLib           PlatformLibDir "/message"
#define MessageRenderingLib  PlatformLibDir "/render_message"
#define ObjectExpansionLib   PlatformLibDir "/expand_object"
#define ObjectLib            PlatformLibDir "/object"
#define ProgramLib           PlatformLibDir "/program"
#define RenderLib            PlatformLibDir "/render"
#define RodneyClientLib      PlatformLibDir "/rodney_client"
#define SessionLib           PlatformLibDir "/session"
#define SqlLib               PlatformLibDir "/sql"
#define StringsLib           PlatformLibDir "/strings"
#define StructLib            PlatformLibDir "/struct"
#define UserLib              PlatformLibDir "/user"
#define ValidationLib        PlatformLibDir "/validation"
#define ZoneLib              PlatformLibDir "/zone"

#define AvatarMixin          PlatformModuleDir "/avatar"
#define CommandCode          PlatformModuleDir "/command"
#define CommandGiverMixin    PlatformModuleDir "/command_giver"
#define PropertyMixin        PlatformModuleDir "/property"
#define SensorMixin          PlatformModuleDir "/sensor"
#define SoulMixin            PlatformModuleDir "/soul"
#define SqlMixin             PlatformModuleDir "/sql"

#define CommandController    PlatformModuleDir "/command_controller"
#define ZoneController       PlatformModuleDir "/zone_controller"

#define AccessService        PlatformObjDir "/access_service"
#define HookService          PlatformObjDir "/hook_service"
#define PostalService        PlatformObjDir "/postal_service"
#define TrackerService       PlatformObjDir "/tracker_service"

#define LoggerFactory        PlatformObjDir "/logger/logger_factory"
#define SqlClientFactory     PlatformObjDir "/sql/sql_client_factory"

#define LoginObject          PlatformObjDir "/login"
#define PlatformAvatar       PlatformObjDir "/avatar"
#define PlatformController   PlatformObjDir "/platform_controller"

#define PlatformTrackerDir   PlatformObjDir "/tracker"
#define ConnectionTracker    PlatformTrackerDir "/connection"
#define DomainTracker        PlatformTrackerDir "/domain"
#define FileTracker          PlatformTrackerDir "/file"
#define FlavorTracker        PlatformTrackerDir "/flavor"
#define ObjectTracker        PlatformTrackerDir "/object"
#define PlayerTracker        PlatformTrackerDir "/player"
#define ProgramTracker       PlatformTrackerDir "/program"
#define SessionTracker       PlatformTrackerDir "/session"
#define TopicTracker         PlatformTrackerDir "/topic"
#define UserTracker          PlatformTrackerDir "/user"
#define ZoneTracker          PlatformTrackerDir "/zone"

#define PlatformRenderDir    PlatformObjDir "/render"
#define DefaultRenderer      PlatformRenderDir "/default"
#define LoginRenderer        PlatformRenderDir "/login"

#endif  // _PLATFORM_H
