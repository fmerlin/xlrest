#include "stdafx.h"
#include "CppUnitTest.h"
#include "JsonObject.h"
#include "Operation.h"
#include "HttpServer.h"
#include "Config.h"
#include "Parameter.h"
#include "XlOperTable.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace xlrestapi
{		

	/*
	TEST_CLASS(Configs)
	{
	public:

		TEST_METHOD(TestIni)
		{
			rms::Config cfg;
			cfg.init3(nullptr);
			//issue in destructor since it calls/destroy the singleton rms::config instead of the object created
		}
	};
	*/

	TEST_CLASS(JsonObjects)
	{
	public:
		
		TEST_METHOD(TestConstructor)
		{
			rms::JsonInt * joi = new rms::JsonInt(1);
			Assert::AreEqual(joi->toString(),std::string("1"));

			rms::JsonDouble * jod = new rms::JsonDouble(1.0);
			Assert::AreEqual(jod->toString().substr(0,3), std::string("1.0"));


			rms::JsonString * jos = new rms::JsonString(std::string("key1"));
			Assert::AreEqual(jos->toString(), std::string("key1"));

			rms::JsonMap * jom = new rms::JsonMap(joi);
			jom->add(jos);
			jom->put(std::string("jod"), jod);


			std::stringstream os;
			jom->write(os);
			Assert::AreEqual(os.str(), std::string("{\"jod\":1}"));
			

			rms::JsonList * jol = new rms::JsonList(joi);
			jol->add(jos);
			jol->add(jos);
			jol->add(jos);
			
			std::stringstream osl;
			jol->write(osl);
			Assert::AreEqual(osl.str(), std::string("[\"key1\",\"key1\",\"key1\"]"));


		}

		TEST_METHOD(TestJsonids)
		{
			
			rms::JsonId joi("true");
			rms::JsonId joi2("false");
			rms::JsonId joi3("null");
			rms::JsonId joi4("nan");

			try
			{
				rms::JsonId joi4("hello");
				Assert::Fail();
			}
			catch(std::exception e)
			{
				
			}

		}

	};


	std::shared_ptr<rms::JsonObject> parse(std::string & msg)
	{
		rms::Tokenizer tk;
		rms::JsonObject* res = NULL;
		strcpy_s(tk.buffer, msg.c_str());
		tk.max = msg.length();
		tk.next();
		tk.max = 1;
		tk.buffer[0] = 0;
		tk.next();
		return std::shared_ptr<rms::JsonObject>(tk.getRoot());
	}

	TEST_CLASS(Tokenizers)
	{
	public:

		TEST_METHOD(TestParseMap)
		{
			std::string msg("{\"jé@\":1}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::stringstream os;
			res->write(os);
			Assert::AreEqual(os.str(), msg);
		}


		TEST_METHOD(TestParseList)
		{
			std::string msg("[\"key1\",\"key1\",\"key1\"]");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::stringstream os;
			res->write(os);
			Assert::AreEqual(os.str(), msg);
		}

			
		TEST_METHOD(TestParseComplex)
		{
			std::string msg("{\"menu\":{\"id\":\"file\",\"popup\":{\"menuitem\":[{\"onclick\":\"CreateNewDoc()\",\"value\":\"New\"},{\"onclick\":\"OpenDoc()\",\"value\":\"Open\"},{\"onclick\":\"CloseDoc()\",\"value\":\"Close\"}]},\"value\":\"File\"}}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::stringstream os;
			res->write(os);
			Assert::AreEqual(os.str(), msg);
		}

		TEST_METHOD(TestScientific)
		{
			std::string msg("{\"EXPOVAL\":-5.779845557258532E-4}");
			std::string expected("{\"EXPOVAL\":-0.000577985}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::stringstream os;
			res->write(os);
			Assert::AreEqual(os.str(), expected);
		}
	};


	std::shared_ptr<rms::HttpServer> createServer()
	{
		std::string server_config("{\"active\":true,\"context\":\"context\",\"host\":\"rms.gdfsuez.net\",\"port\":8310,\"use_https\":false,\"use_proxy\":false,\"tags\":[\"EXCEL\"]}");
		std::shared_ptr<rms::JsonObject> res_server = parse(server_config);

		std::stringstream os_server;
		res_server->write(os_server);
		Assert::AreEqual(os_server.str(), server_config);

		rms::HttpServer * server = new rms::HttpServer(static_cast<rms::JsonMap * >(res_server.get()), false);

		return std::shared_ptr<rms::HttpServer>(server);
	}


	TEST_CLASS(Parameters)
	{
	public:

		TEST_METHOD(TestPath)
		{
			std::string msg("{\"name\":\"label\",\"in\":\"path\",\"description\":\"nameofacacheontheserver\",\"required\":true,\"type\":\"string\"}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::shared_ptr<rms::HttpServer> server = createServer();

			rms::Parameter * p = rms::Parameter::create(static_cast<rms::JsonMap *>(res.get()), server.get());

			Assert::AreEqual((int)p->getLocation(), (int)rms::Location::PATH);
			Assert::AreEqual(p->getName(), std::string("label"));
			Assert::AreEqual(p->getDescription(), std::string("nameofacacheontheserver"));

			std::string msgv("\"MTM\"");
			std::shared_ptr<rms::JsonObject> resv = parse(msgv);

			XLOPER12 xloper;
			p->toXlOper(resv.get(), &xloper);
			Assert::AreEqual((int)xloper.xltype, (int)xltypeStr);
			Assert::AreEqual(xloper.val.str+1, L"MTM");

		}

		TEST_METHOD(TestQuery)
		{
			std::string msg("{\"name\":\"label\",\"in\":\"query\",\"description\":\"nameofacacheontheserver\",\"required\":true,\"type\":\"string\"}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::shared_ptr<rms::HttpServer> server = createServer();

			rms::Parameter * p = rms::Parameter::create(static_cast<rms::JsonMap *>(res.get()), server.get());

			Assert::AreEqual((int)p->getLocation(), (int)rms::Location::QUERY);
			Assert::AreEqual(p->getName(), std::string("label"));
			Assert::AreEqual(p->getDescription(), std::string("nameofacacheontheserver"));

			std::string msgv("\"6\"");
			std::shared_ptr<rms::JsonObject> resv = parse(msgv);

			XLOPER12 xloper;
			p->toXlOper(resv.get(), &xloper);
			Assert::AreEqual((int)xloper.xltype, (int)xltypeStr);
			Assert::AreEqual(xloper.val.str+1, L"6");
		}


		TEST_METHOD(TestBody)
		{
			std::string msg("{\"name\":\"label\",\"in\":\"body\",\"description\":\"nameofacacheontheserver\",\"required\":true,\"type\":\"integer\"}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::shared_ptr<rms::HttpServer> server = createServer();

			rms::Parameter * p = rms::Parameter::create(static_cast<rms::JsonMap *>(res.get()), server.get());

			Assert::AreEqual((int)p->getLocation(), (int)rms::Location::BODY);
			Assert::AreEqual(p->getName(), std::string("label"));
			Assert::AreEqual(p->getDescription(), std::string("nameofacacheontheserver"));

			std::string msgv("16");
			std::shared_ptr<rms::JsonObject> resv = parse(msgv);

			rms::JsonObject * o = resv.get();

			XLOPER12 xloper;
			p->toXlOper(o, &xloper);
			Assert::AreEqual((int)xloper.xltype, (int)xltypeInt);
			Assert::AreEqual(xloper.val.num, 16.0);
		}


		TEST_METHOD(TestDate)
		{
			std::string msg("{\"name\":\"label\",\"in\":\"path\",\"description\":\"nameofacacheontheserver\",\"required\":true,\"type\":\"string\",\"x-example\":\"2001-01-01\"}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::shared_ptr<rms::HttpServer> server = createServer();

			rms::Parameter * p = rms::Parameter::create(static_cast<rms::JsonMap *>(res.get()), server.get());

			std::string firstjan2016 = p->toStringD(42370.0);
			Assert::AreEqual(firstjan2016, std::string("2016-01-01T00:00:00"));

			Assert::AreEqual((int)p->getLocation(), (int)rms::Location::PATH);
			Assert::AreEqual(p->getName(), std::string("label"));
			Assert::AreEqual(p->getDescription(), std::string("nameofacacheontheserver"));

			std::string msgv("{\"res\":\"2016-01-01\"}");
			std::shared_ptr<rms::JsonObject> resv = parse(msgv);

			rms::JsonObject * o = (static_cast<rms::JsonMap *>(resv.get()))->get("res");

			XLOPER12 xloper;
			p->toXlOper(o, &xloper);
			Assert::AreEqual((int)xloper.xltype, (int)xltypeNum);
			Assert::AreEqual(xloper.val.num, 42370.0);


			std::string msgbe("{\"res\":\"2016-11-30T24:00:00\"}");
			std::shared_ptr<rms::JsonObject> resbe = parse(msgbe);
			rms::JsonObject * obe = (static_cast<rms::JsonMap *>(resbe.get()))->get("res");

			XLOPER12 xloperbe;
			p->toXlOper(obe, &xloperbe);
			Assert::AreEqual((int)xloperbe.xltype, (int)xltypeNum);
			Assert::AreEqual(xloperbe.val.num, 42705.0);


		}

		TEST_METHOD(TestList)
		{
			std::string msg("{\"name\":\"values\",\"in\":\"formData\",\"description\":\"values\",\"required\":true,\"type\":\"array\",\"items\":{\"type\":\"number\",\"format\":\"double\"},\"collectionFormat\":\"multi\"}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::shared_ptr<rms::HttpServer> server = createServer();

			rms::Parameter * p = rms::Parameter::create(static_cast<rms::JsonMap *>(res.get()), server.get());
		
			Assert::AreEqual((int)p->getLocation(), (int)rms::Location::FORMDATA);
		}

	};



	TEST_CLASS(Operations)
	{
	public:

		TEST_METHOD(TestOp)
		{
			std::string msg("{\"description\":\"\",\"operationId\":\"auditDeal\",\"parameters\":[{\"default\":\"PROD\",\"in\":\"query\",\"name\":\"label\",\"required\":false,\"type\":\"string\"},{\"in\":\"query\",\"name\":\"refdate\",\"required\":false,\"type\":\"string\"},{\"in\":\"path\",\"name\":\"name\",\"required\":true,\"type\":\"string\"}],\"produces\":[\"application / octet - stream\"],\"responses\":{\"default\":{\"description\":\"successful operation\"}},\"summary\":\"Retrieve xls document with detailed computation of all the buckets for the specified deal\",\"tags\":[\"EXCEL\"]}");
			std::shared_ptr<rms::JsonObject> res = parse(msg);

			std::stringstream os;
			res->write(os);
			Assert::AreEqual(os.str(), msg);

			std::shared_ptr<rms::HttpServer> server = createServer();


			rms::Operation * op = new rms::Operation(server.get(), std::string("get"), std::string("url"), static_cast<rms::JsonMap *>(res.get()));
			//no response 200 defined
 			Assert::AreEqual(op->isActive(), false);



			std::string msg2("{\"description\":\"\",\"operationId\":\"auditDeal\",\"parameters\":[{\"default\":\"PROD\",\"in\":\"query\",\"name\":\"label\",\"required\":false,\"type\":\"string\"},{\"in\":\"query\",\"name\":\"refdate\",\"required\":false,\"type\":\"string\"},{\"in\":\"path\",\"name\":\"name\",\"required\":true,\"type\":\"string\"}],\"produces\":[\"application/json\"],\"responses\":{\"200\":{\"description\":\"successful operation\",\"schema\":{\"type\":\"string\"}}},\"summary\":\"Retrieve xls document with detailed computation of all the buckets for the specified deal\",\"tags\":[\"EXCEL\"]}");
			std::shared_ptr<rms::JsonObject> res2 = parse(msg2);

			std::stringstream os2;
			res2->write(os2);
			Assert::AreEqual(os2.str(), msg2);


			rms::Operation * op2 = new rms::Operation(server.get(), std::string("get"), std::string("url"), static_cast<rms::JsonMap *>(res2.get()));
			Assert::AreEqual(op2->isActive(), true);


		}

	};

	TEST_CLASS(XlOperTable)
	{
	public:
		TEST_METHOD(TestToTable)
		{
			rms::JsonMap * jom = new rms::JsonMap(NULL);
			rms::JsonList * jol = new rms::JsonList(jom);
			jom->put("name",new rms::JsonString("X"));
			jom->put("list", jol);
			jol->add(new rms::JsonInt(1));
			jol->add(new rms::JsonInt(3));
			jol->add(new rms::JsonInt(5));
			rms::XlOperTable* table = jom->toTable();
			Assert::AreEqual(table->size(),3);
			delete jom;
		}

		TEST_METHOD(TestColumnOrder)
		{
			rms::JsonList * jol = new rms::JsonList(NULL);
			rms::JsonMap * jom1 = new rms::JsonMap(jol);
			jom1->put("A", new rms::JsonString("X"));
			jom1->put("B", new rms::JsonString("X"));
			rms::JsonMap * jom2 = new rms::JsonMap(jol);
			jom2->put("B", new rms::JsonString("X"));
			jom2->put("C", new rms::JsonString("X"));
			rms::JsonMap * jom3 = new rms::JsonMap(jol);
			jom3->put("A", new rms::JsonString("X"));
			jom3->put("C", new rms::JsonString("X"));
			jol->add(jom2);
			jol->add(jom3);
			jol->add(jom1);
			rms::XlOperTable* table = jol->toTable();
			table->clean();
			table->incCol();
			std::map<std::string, int> cols;
			for (rms::XlOperColumns::const_iterator it = table->begin(); it != table->end(); it++)
				cols[it->first] = it->second->getCol();
			Assert::AreEqual(1, cols["A"]);
			Assert::AreEqual(2, cols["B"]);
			Assert::AreEqual(3, cols["C"]);
			delete jol;
		}
	};
}