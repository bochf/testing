#include <iostream>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <grpc++/grpc++.h>

#include "clio_api.pb.h"
#include "clio_api.grpc.pb.h"

using namespace std;
using namespace grpc;
using namespace ClioApi;

class ClioServerImpl final : public ClioApi::ClioServer::Service {
	public:
		Status Shutdown(ServerContext * context,
				const A2C_ShutdownRequest * request,
				C2A_ShutdownResponse * response) override {
			cout << request->GetDescriptor()->full_name() << endl;
			m_server->Shutdown();
			return Status::OK;
		}
		Status Suspend(ServerContext * context,
				const A2C_SuspendRequest * request,
				C2A_SuspendResponse * response) override {
			cout << request->GetDescriptor()->full_name() << endl;
			return Status::OK;
		}
		Status Resume(ServerContext * context,
				const A2C_ResumeRequest * request,
				C2A_ResumeResponse * response) override {
			cout << request->GetDescriptor()->full_name() << endl;
			return Status::OK;
		}

		// Start service
		void Run() {
			string server_address("0.0.0.0:51234");
			ServerBuilder builder;

			// listen on the given address
			builder.AddListeningPort(server_address, InsecureServerCredentials());
			// register "service" as the instance through which we'll communicate with clients.
			// in this case, it corresponds to an *synchronous* service.
			builder.RegisterService(this);

			// finally assemble the server.
			m_server = builder.BuildAndStart();
			cout << "Clio Server is listening on " << server_address << endl;

			// wait for the server to shutdown.
			m_server->Wait();
		}

	private:
		unique_ptr<Server> m_server;
};

TEST(ProtoBuf, gRPC) {
	ClioServerImpl clio_server;
	clio_server.Run();
}

GTEST_API_ int main(int argc, char ** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
