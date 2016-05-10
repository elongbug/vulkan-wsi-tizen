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
BuildRequires: Vulkan-LoaderAndValidationLayers

#%define _unpackaged_files_terminate_build 0
%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}

%description
Vulkan WSI (Window System Integration) Layer for Tizen

%package samples
Summary:	Vulkan sample
Group:		Graphics & UI Framework/Hardware Adaptation
Requires:	%{name} = %{version}-%{release}

%package devel
Summary:	Development package for tizen vulkan driver
Group:		Graphics & UI Framework/Hardware Adaptation
Requires:	%{name} = %{version}-%{release}

%description samples
Vulkan WSI (Window System Integration) sample with null-driver for Test

%description devel
Development packages for tizen vulkan driver

%prep
%setup -q

%build
%autogen
make %{?_smp_mflags}

%install
%make_install
mkdir -p %{buildroot}/%{TZ_SYS_RO_SHARE}/license
mkdir -p %{buildroot}/%{_bindir}
cp -a %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{TZ_SYS_RO_SHARE}/license/%{name}
cp %{_builddir}/%{buildsubdir}/samples/tri %{buildroot}/%{_bindir}
cp %{_builddir}/%{buildsubdir}/samples/vulkaninfo %{buildroot}/%{_bindir}

%files -n %{name}
%{TZ_SYS_RO_SHARE}/license/%{name}
%defattr(-,root,root,-)
%{_libdir}/vulkan/vulkan-wsi-tizen.so
/etc/vulkan/icd.d/vulkan-wsi-tizen.json
%manifest packaging/vulkan-wsi-tizen.manifest

%files samples
%{_libdir}/vulkan/null-driver.so
%{_bindir}/tri
%{_bindir}/vulkaninfo

%files devel
%defattr(-,root,root,-)
%{_includedir}/vulkan/vulkan-wsi-tizen.h
