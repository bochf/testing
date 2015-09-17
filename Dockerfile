#build base image for Clio service

FROM centos:7

RUN yum -y install tar
RUN yum -y install wget
RUN yum -y install rsync
RUN yum -y install patch

# INSTALL JAVA  jdk-8u60-linux-x64.tar.gz
RUN cd /opt && wget --no-cookies --no-check-certificate --header "Cookie: gpw_e24=http%3A%2F%2Fwww.oracle.com%2F; oraclelicense=accept-securebackup-cookie" "http://download.oracle.com/otn-pub/java/jdk/8u60-b27/jdk-8u60-linux-x64.tar.gz" \
   && tar xzf jdk-8u60-linux-x64.tar.gz && rm -rf jdk-8u60-linux-x64.tar.gz

ENV JAVA_HOME /opt/jdk1.8.0_60


ENV TOMCAT_MAJOR_VERSION 7
ENV TOMCAT_MINOR_VERSION 7.0.55
ENV CATALINA_HOME /usr/local/tomcat7

# INSTALL TOMCAT
RUN cd /tmp && wget -q https://archive.apache.org/dist/tomcat/tomcat-${TOMCAT_MAJOR_VERSION}/v${TOMCAT_MINOR_VERSION}/bin/apache-tomcat-${TOMCAT_MINOR_VERSION}.tar.gz && \
    wget -qO- https://archive.apache.org/dist/tomcat/tomcat-${TOMCAT_MAJOR_VERSION}/v${TOMCAT_MINOR_VERSION}/bin/apache-tomcat-${TOMCAT_MINOR_VERSION}.tar.gz.md5 | md5sum -c - && \
    cd /usr/local && /bin/tar zxf /tmp/apache-tomcat-*.tar.gz && \
    rm /tmp/apache-tomcat-*.tar.gz && \
    mv apache-tomcat* tomcat7

RUN cd /tmp && wget http://mirrors.gigenet.com/apache/maven/maven-3/3.3.3/binaries/apache-maven-3.3.3-bin.tar.gz
RUN cd /usr/local && /bin/tar -zxvf /tmp/apache-maven-3.3.3-bin.tar.gz && rm /tmp/apache-maven-3.3.3-bin.tar.gz

ENV M2_HOME /usr/local/apache-maven-3.3.3
ENV PATH $JAVA_HOME/bin:$M2_HOME/bin:$PATH

RUN cd /usr/local/tomcat7/webapps && rm -rf manager && rm -rf host-manager && rm -rf examples && rm -rf docs && rm -rf ROOT

#####################################################################################################
# install c++ development tool chain and necessary libraries
RUN yum install -y epel-release
RUN yum install -y \
    autoconf automake gcc gcc-c++ gdb \
    git glibc glibc-devel libtool \
    lcov doxygen \
    man pkgconfig python python-pip python-devel \
    systemtap systemtap-devel \
    unzip vim-enhanced which bzip2 bzip2-devel bzip2-libs zlib zlib-devel tcpdump

#####################################################################################################
# install Boost
# get Boost source code
RUN cd /opt && wget http://skylineservers.dl.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.gz && tar -xzvf boost_1_59_0.tar.gz
# compile and install Boost library
RUN cd /opt/boost_1_59_0 && ./bootstrap.sh --prefix=/usr && ./b2 install

#####################################################################################################
# install protobuf
# get protobuf source code
RUN cd /opt && git clone https://github.com/google/protobuf.git
# compile and install protobuf
RUN cd /opt/protobuf && ./autogen.sh && ./configure --prefix=/usr && make && make install
# setup pkg-config search path for protobuf
ENV PKG_CONFIG_PATH /usr/lib/pkgconfig
# add library search path for protobuf
RUN echo "/usr/lib" > /etc/ld.so.conf.d/protobuf.conf
RUN ldconfig
# build protobuf Java library
RUN cd /opt/protobuf/java && mvn install
ENV CLASSPATH ${CLASSPATH}:/opt/protobuf/java/target/protobuf-java-3.0.0-beta-1.jar
# build protobuf Python library
RUN cd /opt/protobuf/python && pip install pbr && python setup.py build && python setup.py install

#####################################################################################################
# install gRPC
# get gRPC source code
RUN cd /opt && git clone https://github.com/grpc/grpc.git
# compile and install gRPC
RUN cd /opt/grpc && git submodule update --init && make && make install

CMD ["/bin/bash"]
