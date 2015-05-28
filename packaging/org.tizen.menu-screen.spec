%define _optdir /opt
%define _usrdir /usr
%define _appdir %{_usrdir}/apps
%define _usr_datadir %{_usrdir}/share

%define _project_name menu-screen
%define _package_name org.tizen.%{_project_name}

%define _packagedir %{_appdir}/%{_package_name}
%define _bindir %{_packagedir}/bin
%define _datadir %{_optdir}%{_packagedir}/data
%define _resdir %{_packagedir}/res
%define _sharedir %{_packagedir}/share

%define _localedir %{_resdir}/locale

Name:       org.tizen.menu-screen
Summary:    An utility library of the menu screen
Version:    1.2.3
Release:    1.1
Group:      TO_BE/FILLED_IN
License:    Flora Software License
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(ail)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(badge)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(heynoti)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(shortcut)
BuildRequires:  pkgconfig(sysman)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools


%description
An utility library for developers of the menu screen.



%package devel
Summary:    An utility library of the menu screen (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
An utility library for developers of the menu screen (devel)


%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
CFLAGS="${CFLAGS} -Wall -Werror" LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed"
make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}%{_datadir}
mkdir -p %{buildroot}/usr/share/license

%post
INHOUSE_ID="5000"

init_vconf()
{
	vconftool set -t int memory/idle-screen/top 0 -i -u 5000 -f
	vconftool set -t string file/private/org.tizen.menu-screen/engine "gl" -i -u 5000 -f
	vconftool set -t string db/setting/menuscreen/package_name "org.tizen.menu-screen" -i -u 5000 -f
	vconftool set -t bool db/private/org.tizen.menu-screen/app_relay 0 -g 5000
}
init_vconf

if [ ! -d %{_datadir}/dbspace ]
then
	mkdir -p %{_datadir}/dbspace
fi

if [ ! -d %{_datadir}/shortcut ]
then
	mkdir -p %{_datadir}/shortcut
else
	rm -rf %{_datadir}/shortcut/*
fi

sqlite3 %{_datadir}/dbspace/.menu_screen.db 'PRAGMA journal_mode = PERSIST;
	create table if not exists shortcut (
		ROWID INTEGER PRIMARY KEY AUTOINCREMENT,
		appid TEXT,
		name TEXT,
		type INTEGER,
		content_info TEXT,
		icon TEXT
	);
'

INHOUSE_ID="5000"
chown -R $INHOUSE_ID:$INHOUSE_ID %{_datadir}
chown root:$INHOUSE_ID %{_datadir}/dbspace/.menu_screen.db
chown root:$INHOUSE_ID %{_datadir}/dbspace/.menu_screen.db-journal

chmod 660 %{_datadir}/dbspace/.menu_screen.db
chmod 660 %{_datadir}/dbspace/.menu_screen.db-journal

if [ -f /usr/lib/rpm-plugins/msm.so ]
then
	chsmack -a 'org.tizen.menu-screen' %{_datadir}/dbspace/.menu_screen.db*
fi

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_bindir}/menu-screen
%{_resdir}/edje/all_apps_portrait.edj
%{_resdir}/edje/button_edit.edj
%{_resdir}/edje/group_4x4_portrait.edj
%{_resdir}/edje/item_4x4.edj
%{_resdir}/edje/layout_portrait.edj
%{_resdir}/edje/index.edj
%{_resdir}/images/default.png
%{_localedir}/*/*/*.mo
%{_usr_datadir}/packages/org.tizen.menu-screen.xml
%{_usr_datadir}/license/%{name}
/opt/etc/smack/accesses.d/%{_package_name}.rule
