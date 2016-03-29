Name:		vulkan-wsi-tizen
Version:	1.0.0
Release:	0
Summary:	Vulkan WSI (Window System Integration) Layer for Tizen
License:	MIT
Group:		Graphics & UI Framework/Hardware Adaptation

Source:		%{name}-%{version}.tar.gz

BuildRequires: autoconf > 2.64
BuildRequires: automake >= 1.11
BuildRequires: libtool >= 2.2

%description
Vulkan WSI (Window System Integration) Layer for Tizen

%prep
%setup -q

%build
%autogen
make %{?_smp_mflags}

%install
%make_install

%files -n %{name}
%defattr(-,root,root,-)
%{_libdir}/vulkan/vulkan-wsi-tizen.so
/etc/vulkan/icd.d/vulkan-wsi-tizen.json
%manifest packaging/vulkan-wsi-tizen.manifest
