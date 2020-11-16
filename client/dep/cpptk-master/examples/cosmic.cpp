#include "cpptk.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>

int main(int, char *argv[])
{
	Tk::init(argv[0]);

	//First tutorial
	/*Tk::button(".b") -text("Hello World");
	Tk::grid(Tk::configure,".b");*/

	//Second tutorial
	/*Tk::wm(Tk::title, ".", "Feet to meters"); //Change the title of the main window
	Tk::frame(".c") -Tk::padx(3) -Tk::pady(12); //FIXME: How to support padding("3 3 12 12")?
	Tk::grid(Tk::configure, ".c") -Tk::column(3) -Tk::row(2) -Tk::sticky("nsew");
	Tk::grid(Tk::columnconfigure, ".", 0) -Tk::weight(1);
	Tk::grid(Tk::rowconfigure, ".", 0) -Tk::weight(1);

	double feet;
	Tk::entry(".c.feet") -Tk::width(7) -Tk::textvariable(feet);
	Tk::grid(Tk::configure, ".c.feet") -Tk::column(2) -Tk::row(1) -Tk::sticky("we");

	double meters;
	Tk::label(".c.meters") -Tk::textvariable(meters);
	Tk::grid(Tk::configure, ".c.meters") -Tk::column(2) -Tk::row(2) -Tk::sticky("we");
	std::function<void()> calculate = [&] { meters = 0.3048*feet; };
	Tk::button(".c.calc") -Tk::text("Calculate") -Tk::command(calculate);
	Tk::grid(Tk::configure, ".c.calc") -Tk::column(3) -Tk::row(3) -Tk::sticky("w");

	Tk::label(".c.flbl") -Tk::text("feet");
	Tk::label(".c.islbl") -Tk::text("is equivalent to");
	Tk::label(".c.mlbl") -Tk::text("meters");
	Tk::grid(Tk::configure, ".c.flbl") -Tk::column(3) -Tk::row(1) -Tk::sticky("w");
	Tk::grid(Tk::configure, ".c.islbl") -Tk::column(1) -Tk::row(2) -Tk::sticky("e");
	Tk::grid(Tk::configure, ".c.mlbl") -Tk::column(3) -Tk::row(2) -Tk::sticky("w");

	Tk::grid(Tk::configure, Tk::winfo(Tk::children, ".c")) - Tk::padx(5) -Tk::pady(5);
	Tk::focus(".c.feet");
	Tk::bind(".", "<Return>", calculate);*/

	//Configuration options
	/*std::function<void()> button_pressed = [] {};
	Tk::button(".b") -Tk::text("Hello") -Tk::command(button_pressed);

	std::string button_text(".b" << Tk::cget(Tk::text));
	std::string command(".b" << Tk::cget(Tk::command));
	std::cout << "Button text: " << button_text << "\n";
	std::cout << "Button cmd: " << command << "\n";
	".b" << Tk::configure() -Tk::text("Goodbye");
	button_text = std::string(".b" << Tk::cget(Tk::text));
	std::cout << "Button text: " << button_text << "\n";*/

	//Binding to events
	/*Tk::label(".l") -Tk::text("Starting...");
	Tk::grid(Tk::configure, ".l");
	Tk::bind(".l", "<Enter>", [] () { ".l" << Tk::configure() -Tk::text("Moved mouse inside"); });
	Tk::bind(".l", "<Leave>", [] () { ".l" << Tk::configure() -Tk::text("Moved mouse outside"); }); 
	Tk::bind(".l", "<ButtonPress-1>", [] () { ".l" << Tk::configure() -Tk::text("Clicked left mouse button"); }); 
	Tk::bind(".l", "<3>", [] () { ".l" << Tk::configure() -Tk::text("Clicked right mouse button"); }); 
	Tk::bind(".l", "<Double-1>", [] () { ".l" << Tk::configure() -Tk::text("Double clicked left mouse button"); });
	Tk::bind(".l", "<B3-Motion>", [] (int x, int y) { std::stringstream msg; msg << "Right button drag to " << x << " " << y; ".l" << Tk::configure() -Tk::text(msg.str()); }, Tk::event_x, Tk::event_y);*/

	//Validation of entries
	/*std::string zip;
	Tk::entry(".e") -Tk::textvariable(zip) -Tk::validate(Tk::key) -Tk::validatecommand([] (const std::string &n) { return std::all_of(n.begin(),n.end(),::isdigit) && n.size() <= 5; }, Tk::valid_P);
	Tk::grid(Tk::configure, ".e") -Tk::column(0) -Tk::row(0) -Tk::sticky("we");*/

	//Advanced validation example
	/*Tk::frame(".f");
	Tk::grid(Tk::configure, ".f") -Tk::column(0) -Tk::row(0);
	Tk::label(".f.l1") -Tk::text("Name:");
	Tk::grid(Tk::configure, ".f.l1") -Tk::column(0) -Tk::row(0) -Tk::padx(5) -Tk::pady(5);
	Tk::entry(".f.e1");
	Tk::grid(Tk::configure, ".f.e1") -Tk::column(1) -Tk::row(0) -Tk::padx(5) -Tk::pady(5);
	Tk::label(".f.l") -Tk::text("Zip:");
	Tk::grid(Tk::configure, ".f.l") -Tk::column(0) -Tk::row(1) -Tk::padx(5) -Tk::pady(5);

	std::string zip;
	std::string errmsg;
	auto callback = [&errmsg] (const std::string &newval, const std::string &op)
	{
		errmsg = std::string("");
		bool valid = false;
		std::string first_zip;
		if(newval.size() >= 5)
		{
			first_zip = std::string(newval.begin(),newval.begin()+5);
		}
		else
		{
			valid = false;
		}
		if(newval.size() == 5 && std::all_of(first_zip.begin(),first_zip.end(),::isdigit))
		{
			valid = true;
		}
		std::string second_zip;
		if(newval.size() == 10)
		{
			second_zip = std::string(newval.begin()+6,newval.end());
			if(std::all_of(first_zip.begin(),first_zip.end(),::isdigit) && std::all_of(second_zip.begin(),second_zip.end(),::isdigit) && newval[5] == '-')
			{
				valid = true;
			}
		}

		if(valid)
		{
			".f.btn" << Tk::configure() -Tk::state(Tk::normal);
		}
		else
		{
			".f.btn" << Tk::configure() -Tk::state(Tk::disabled);
		}

		if(op.compare("key") == 0)
		{
			//Check if valid so far
			bool ok_so_far = false;
			if(newval.size() <= 10)
			{
				if(std::all_of(newval.begin(),newval.end(),[] (char c) { if((c >= '0' && c <= '9') || c == '-') return true; return false; }))
				{
					ok_so_far = true;
				}
			}

			if(!ok_so_far)
			{
				errmsg = "Zip should be ##### or #####-####";
			}
			return ok_so_far;
		}
		else if(op.compare("focusout") == 0)
		{
			//Send error if not valid
			if(!valid)
			{
				errmsg = "Zip should be ##### or #####-####";
			}
		}

		return valid;
	};

	Tk::entry(".f.e") -Tk::textvariable(zip) -Tk::validate(Tk::all) -Tk::validatecommand(callback, Tk::valid_P, Tk::valid_V);
	Tk::grid(Tk::configure,".f.e") -Tk::column(1) -Tk::row(1) -Tk::padx(5) -Tk::pady(5);
	Tk::button(".f.btn") -Tk::text("Process");
	Tk::grid(Tk::configure, ".f.btn") -Tk::column(2) -Tk::row(1) -Tk::padx(5) -Tk::pady(5);
	".f.btn" << Tk::configure() -Tk::state(Tk::disabled);
	Tk::label(".f.msg") -Tk::font("SmallCaptionFont") -Tk::foreground("red") -Tk::textvariable(errmsg);
	Tk::grid(Tk::configure, ".f.msg") -Tk::column(1) -Tk::row(2) -Tk::padx(5) -Tk::pady(5) -Tk::sticky("w");*/

	//Grid spanning multiple cells
	/*int one = 1;
	int two = 0;
	int three = 1;
	Tk::frame(".c");
	Tk::frame(".c.f") -Tk::borderwidth(5) -Tk::relief(Tk::ridge) -Tk::width(200) -Tk::height(100);
	Tk::label(".c.namelbl") -Tk::text("Name");
	Tk::entry(".c.name");
	Tk::checkbutton(".c.one") -Tk::text("One") -Tk::variable(one) -Tk::onvalue(1);
	Tk::checkbutton(".c.two") -Tk::text("Two") -Tk::variable(two) -Tk::onvalue(1);
	Tk::checkbutton(".c.three") -Tk::text("Three") -Tk::variable(three) -Tk::onvalue(1);
	Tk::button(".c.ok") -Tk::text("Okay");
	Tk::button(".c.cancel") -Tk::text("Cancel");

	Tk::grid(Tk::configure, ".c") -Tk::column(0) -Tk::row(0) -Tk::sticky("nsew");
	Tk::grid(Tk::configure, ".c.f") -Tk::column(0) -Tk::row(0) -Tk::columnspan(3) -Tk::rowspan(2) -Tk::sticky("nsew");
	Tk::grid(Tk::configure, ".c.namelbl") -Tk::column(3) -Tk::row(0) -Tk::columnspan(2) -Tk::sticky("nw") -Tk::padx(5);
	Tk::grid(Tk::configure, ".c.name") -Tk::column(3) -Tk::row(1) -Tk::columnspan(2) -Tk::sticky("new") -Tk::pady(5) -Tk::padx(5);
	Tk::grid(Tk::configure, ".c.one") -Tk::column(0) -Tk::row(3);
	Tk::grid(Tk::configure, ".c.two") -Tk::column(1) -Tk::row(3);
	Tk::grid(Tk::configure, ".c.three") -Tk::column(2) -Tk::row(3);
	Tk::grid(Tk::configure, ".c.ok") -Tk::column(3) -Tk::row(3);
	Tk::grid(Tk::configure, ".c.cancel") -Tk::column(4) -Tk::row(3);

	Tk::grid(Tk::columnconfigure, ".", 0) -Tk::weight(1);
	Tk::grid(Tk::rowconfigure, ".", 0) -Tk::weight(1);
	Tk::grid(Tk::columnconfigure, ".c", 0) -Tk::weight(3);
	Tk::grid(Tk::columnconfigure, ".c", 1) -Tk::weight(3);
	Tk::grid(Tk::columnconfigure, ".c", 2) -Tk::weight(3);
	Tk::grid(Tk::columnconfigure, ".c", 3) -Tk::weight(1);
	Tk::grid(Tk::columnconfigure, ".c", 4) -Tk::weight(1);
	Tk::grid(Tk::rowconfigure, ".c", 1) -Tk::weight(1);*/

	//Listbox
	/*class country_info
	{
		public:
		country_info(const std::string &c, const std::string &n, const unsigned p) : code(c), name(n), population(p) {}
		std::string code;
		std::string name;
		unsigned population;
	};

	std::vector<country_info> countries;
	countries.push_back(country_info("ar","Argentina",41000000));
	countries.push_back(country_info("au","Australia",21179211));
	countries.push_back(country_info("be","Belgium",10584534));
	countries.push_back(country_info("br","Brazil",185971537));
	countries.push_back(country_info("ca","Canada",33148682));
	countries.push_back(country_info("cn","China",1323128240));
	countries.push_back(country_info("dk","Denmark",5457415));
	countries.push_back(country_info("fi","Finland",5302000));
	countries.push_back(country_info("fr","France",64102140));
	countries.push_back(country_info("gr","Greece",11147000));
	countries.push_back(country_info("in","India",1131043000));
	countries.push_back(country_info("it","Italy",59206382));
	countries.push_back(country_info("jp","Japan",127718000));
	countries.push_back(country_info("mx","Mexico",106535000));
	countries.push_back(country_info("nl","Netherlands",16402414));
	countries.push_back(country_info("no","Norway",4738085));
	countries.push_back(country_info("es","Spain",45116894));
	countries.push_back(country_info("se","Sweden",9174082));
	countries.push_back(country_info("ch","Switzerland",7508700));

	enum class gifts
	{
		card,
		flowers,
		nastygram
	};

	std::map<gifts,std::string> gift_to_name;
	gift_to_name.insert(std::make_pair(gifts::card,"Greeting card"));
	gift_to_name.insert(std::make_pair(gifts::flowers,"Flowers"));
	gift_to_name.insert(std::make_pair(gifts::nastygram,"Nastygram"));

	int gift = static_cast<int>(gifts::card);
	std::string sentmsg("");
	std::string statusmsg("");

	auto show_population = [&] ()
	{
		std::string idx(".c.countries" << Tk::curselection());
		unsigned index = std::stoi(idx);
		const country_info &country = countries[index];
		std::stringstream tmp;
		tmp << "The population of " << country.name << " (" << country.code << ") is " << country.population;
		statusmsg = tmp.str();
	};

	auto send_gift = [&] ()
	{
		std::string idx(".c.countries" << Tk::curselection());
		unsigned index = std::stoi(idx);
		".c.countries" << Tk::see(index); //If the selected item is out of view, make it visiable
		const country_info &country = countries[index];
		std::stringstream tmp;
		tmp << "Sent " << gift_to_name[static_cast<gifts>(gift)] << " to leader of " << country.name;
		sentmsg = tmp.str();

	};

	Tk::frame(".c") -Tk::padx(5) -Tk::pady(12); //FIXME: We want padding(5,5,12,0) here
	Tk::grid(Tk::configure, ".c") -Tk::column(0) -Tk::row(0) -Tk::sticky("nwes");
	Tk::grid(Tk::columnconfigure, ".", 0) -Tk::weight(1);
	Tk::grid(Tk::rowconfigure, ".", 0) -Tk::weight(1);

	Tk::listbox(".c.countries");
	for(auto i=countries.begin(),e=countries.end();i!=e;++i)
	{
		".c.countries" << Tk::insert(Tk::end,i->name);
	}
	Tk::label(".c.lbl") -Tk::text("Send to country's leader:");
	Tk::radiobutton(".c.g1") -Tk::text(gift_to_name[gifts::card]) -Tk::variable(gift) -Tk::value(static_cast<int>(gifts::card));
	Tk::radiobutton(".c.g2") -Tk::text(gift_to_name[gifts::flowers]) -Tk::variable(gift) -Tk::value(static_cast<int>(gifts::flowers));
	Tk::radiobutton(".c.g3") -Tk::text(gift_to_name[gifts::nastygram]) -Tk::variable(gift) -Tk::value(static_cast<int>(gifts::nastygram));
	Tk::button(".c.send") -Tk::text("Send Gift") -Tk::command(send_gift) -Tk::defaultstate(Tk::active);
	Tk::label(".c.sentlbl") -Tk::textvariable(sentmsg) -Tk::anchor(Tk::center);
	Tk::label(".c.status") -Tk::textvariable(statusmsg) -Tk::anchor(Tk::w);

	Tk::grid(Tk::configure,".c.countries") -Tk::column(0) -Tk::row(0) -Tk::rowspan(6) -Tk::sticky("nsew");
	Tk::grid(Tk::configure,".c.lbl") -Tk::column(1) -Tk::row(0) -Tk::padx(10) -Tk::pady(5);
	Tk::grid(Tk::configure,".c.g1") -Tk::column(1) -Tk::row(1) -Tk::sticky("w") -Tk::padx(20);
	Tk::grid(Tk::configure,".c.g2") -Tk::column(1) -Tk::row(2) -Tk::sticky("w") -Tk::padx(20);
	Tk::grid(Tk::configure,".c.g3") -Tk::column(1) -Tk::row(3) -Tk::sticky("w") -Tk::padx(20);
	Tk::grid(Tk::configure,".c.send") -Tk::column(2) -Tk::row(4) -Tk::sticky("e");
	Tk::grid(Tk::configure,".c.sentlbl") -Tk::column(1) -Tk::row(5) -Tk::columnspan(2) -Tk::sticky("n") -Tk::pady(5) -Tk::padx(5);
	Tk::grid(Tk::configure,".c.status") -Tk::column(0) -Tk::row(6) -Tk::columnspan(2) -Tk::sticky("we");
	Tk::grid(Tk::columnconfigure, ".c", 0) -Tk::weight(1);
	Tk::grid(Tk::rowconfigure, ".c", 5) -Tk::weight(1);

	Tk::bind(".c.countries","<<ListboxSelect>>",show_population);

	Tk::bind(".c.countries","<Double-1>",send_gift);
	Tk::bind(".","<Return>",send_gift);

	for(unsigned i=0; i<countries.size(); i+=2)
	{
		".c.countries" << Tk::itemconfigure(i) -Tk::background(Tk::rgb(0xf0,0xf0,0xff)); //-Tk::background("#f0f0ff") also works
	}

	".c.countries" << Tk::selection(Tk::set,0);*/

	//Scrollbar example
	Tk::listbox(".l") -Tk::yscrollcommand(".s set") -Tk::height(5);
	Tk::grid(Tk::configure,".l") -Tk::column(0) -Tk::row(0) -Tk::sticky("news");
	Tk::scrollbar(".s") -Tk::command(".l yview") -Tk::orient(Tk::vertical);
	Tk::grid(Tk::configure, ".s") -Tk::column(1) -Tk::row(0) -Tk::sticky("ns");
	Tk::label(".stat") -Tk::text("Status message here") -Tk::anchor(Tk::w);
	Tk::grid(Tk::configure, ".stat") -Tk::column(0) -Tk::columnspan(2) -Tk::row(1) -Tk::sticky("we");
	Tk::grid(Tk::columnconfigure, ".", 0) -Tk::weight(1);
	Tk::grid(Tk::rowconfigure, ".", 0) -Tk::weight(1);

	for(unsigned i=0; i<100; i++)
	{
		std::stringstream item;
		item << "Line " << i << " of 100";
		".l" << Tk::insert(Tk::end,item.str());
	}

     	Tk::runEventLoop();
}

