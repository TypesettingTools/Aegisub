#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>

// import EnvDTE
#pragma warning(disable : 4278)
#pragma warning(disable : 4146)
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only amed_guids
#pragma warning(default : 4146)
#pragma warning(default : 4278)

bool visual_studio_open_file(char const * filename, unsigned int line)
{
	HRESULT result;
	CLSID clsid;
	result = ::CLSIDFromProgID(L"VisualStudio.DTE", &clsid);
	if (FAILED(result))
		return false;

	CComPtr<IUnknown> punk;
	result = ::GetActiveObject(clsid, NULL, &punk);
	if (FAILED(result))
		return false;

	CComPtr<EnvDTE::_DTE> DTE;
	DTE = punk;

	CComPtr<EnvDTE::ItemOperations> item_ops;
	result = DTE->get_ItemOperations(&item_ops);
	if (FAILED(result))
		return false;

	CComBSTR bstrFileName(filename);
	CComBSTR bstrKind(EnvDTE::vsViewKindTextView);
	CComPtr<EnvDTE::Window> window;
	result = item_ops->OpenFile(bstrFileName, bstrKind, &window);
	if (FAILED(result))
		return false;

	CComPtr<EnvDTE::Document> doc;
	result = DTE->get_ActiveDocument(&doc);
	if (FAILED(result))
		return false;

	CComPtr<IDispatch> selection_dispatch;
	result = doc->get_Selection(&selection_dispatch);
	if (FAILED(result))
		return false;

	CComPtr<EnvDTE::TextSelection> selection;
	result = selection_dispatch->QueryInterface(&selection);
	if (FAILED(result))
		return false;

	result = selection->GotoLine(line, TRUE);
	if (FAILED(result))
		return false;

	return true;
}
