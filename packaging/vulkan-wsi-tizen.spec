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

BuildRequires: pkgconfig(tpl-egl)
BuildRequires: pkgconfig(libtbm)

%define _unpackaged_files_terminate_build 0
%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}

%description
Vulkan WSI (Window System Integration) Layer for Tizen

%prep
%setup -q

%build
%autogen
make %{?_smp_mflags}

%install
%make_install
mkdir -p %{buildroot}/%{TZ_SYS_RO_SHARE}/license
cp -a %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{TZ_SYS_RO_SHARE}/license/%{name}

%files -n %{name}
%{TZ_SYS_RO_SHARE}/license/%{name}
%defattr(-,root,root,-)
%{_libdir}/vulkan/vulkan-wsi-tizen.so
/etc/vulkan/icd.d/vulkan-wsi-tizen.json
%manifest packaging/vulkan-wsi-tizen.manifest
