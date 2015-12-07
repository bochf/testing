Summary: Localware performance tools
Name: bb-tools-perf
Version: 0.3.0
Release: 1
License: None
Group: BB/Tools
Vendor: Bloomberg LLP
Packager: William Favorite <wfavorite2@bloomberg.net>
BuildArch: x86_64
Buildroot: %{_tmppath}/%{name}-root

%description
This is a collection of G325 localware performance tools written for Linux.

%prep
rm -rf $RPM_BUILD_DIR
mkdir -p $RPM_BUILD_DIR
cp -r $RPM_SOURCE_DIR/vcpu $RPM_BUILD_DIR
cp -r $RPM_SOURCE_DIR/linterrupt $RPM_BUILD_DIR

%build
cd vcpu
make
cd -
cd linterrupt
make
cd -

%install
# Clean up BUILDROOT
rm -rf $RPM_BUILD_ROOT
# Re-build directory structure
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
# Copy in files
cp $RPM_BUILD_DIR/vcpu/vcpu $RPM_BUILD_ROOT/usr/local/bin
cp $RPM_BUILD_DIR/linterrupt/linterrupt $RPM_BUILD_ROOT/usr/local/bin

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR

%files
%defattr(755,root,root)
/usr/local/bin/vcpu
/usr/local/bin/linterrupt

%pre

%postun

