///
/// ROOTVerisonAttribute
///  Attaches the info to an assembly describing the version of ROOT that
/// was used to build this guy against.
///
#pragma once

[System::AttributeUsage(System::AttributeTargets::Assembly)]
public ref class ROOTVersionAttribute : System::Attribute
{
public:
	ROOTVersionAttribute (int svn_version_number,
		System::String ^svn_branch_name,
		System::String ^svn_date,
		System::String ^root_version)
	{
		_svn_version_number = svn_version_number;
		_svn_branch_name = svn_branch_name;
		_svn_date = _svn_date;
		_root_version = root_version;
	}

private:
	int _svn_version_number;
	System::String ^_svn_branch_name;
	System::String ^_svn_date;
	System::String ^_root_version;
};
