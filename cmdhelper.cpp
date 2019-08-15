#include "cmdhelper.h"
#include<unistd.h>
#include<deque>
#include<memory>
#include<list>
#include<signal.h>
#include "minecraft/core/typeid.h"
static void (*dummy)(std::vector<std::string>&,CommandOrigin const &,CommandOutput &outp);
static void mylex(std::string& oper,std::vector<std::string>& out) {
    oper=" "+oper+" ";
    int sz=oper.size();
    const char* c=oper.c_str();
    for(int i=0; i<sz; ++i) {
        if(c[i]==' ')
            for(int j=i+1; j<sz; ++j) {
                if(c[j]==' ') {
                    out.push_back(std::string(c+i+1,j-i-1));
                    break;
                }
            }
    }
}
struct ACmd : Command {
    CommandMessage msg;
    void* callee;
    ~ACmd() override = default;
    void execute(CommandOrigin const &origin, CommandOutput &outp) override
    {
        std::string oper=msg.getMessage(origin);
        std::vector<std::string> args;
        mylex(oper,args);
        ((typeof(dummy))callee)(args,origin,outp);
    }
};
using std::string;
struct req {
    string name,desc;
    void* fn;
};
static std::deque<req> reqs;
void register_cmd(const std::string& name,void* fn,const std::string& desc) {
    reqs.push_back({name,desc,fn});
}
static std::list<void*> onsht;
void register_shutdown(void* fn) {
    onsht.push_back(fn);
}
void call_sht(int d) {
    for(auto i:onsht) {
        ((void(*)())i)();
    }
    if(d==SIGINT) {
        printf("use stop instead!\n");
    }
}
ACmd* chelper(void* fn) {
    ACmd* fk=new ACmd();
    fk->callee=fn;
    return fk;
}
/*
auto dum(){
	return std::unique_ptr<ACmd>(((ACmd*(*)(void*))0xdddddddddddddddd)((void*)0xffffffff77777777));
}*/
void* func_ret_uni(void* a) {
    return wr_ret_uni((u64)a,(char*)chelper);
}
static void handle_regcmd(CommandRegistry& t) {
    for(auto const& i:reqs) {
        t.registerCommand(i.name,i.desc.c_str(),(CommandPermissionLevel)0,(CommandFlag)0,(CommandFlag)32);
        t.registerOverload2(i.name.c_str(),func_ret_uni(i.fn),CommandParameterData(type_id<CommandRegistry, CommandMessage>(), &CommandRegistry::parse<CommandMessage>, "operation", (CommandParameterDataType)0, nullptr, offsetof(ACmd, msg), false, -1));
    }
    reqs.clear();
}
static Minecraft* MC;
Minecraft* getMC() {
    return MC;
}
static void* sss,*ss;
void sss_new(Minecraft& a, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const& d, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const& b, PermissionsFile* c) {
    printf("MC %p\n",&a);
    ((typeof(&sss_new))sss)(a,d,b,c);
    MC=&a;
    return;
}
void ss_new(CommandRegistry& thi,CommandRegistry& a) {
    handle_regcmd(thi);
    ((typeof(&ss_new))ss)(thi,a);
    return;
}
void cmdhelper_init() {
    sss=MyHook(fpm(ServerCommands::setupStandardServer),fp(sss_new));
    ss=MyHook(fpm(SayCommand::setup),fp(ss_new));
#define sh(a) signal(a,call_sht);
    sh(SIGHUP)
    sh(SIGINT)
    sh(SIGILL)
    sh(SIGABRT)
    sh(SIGFPE)
    sh(SIGSEGV)
}