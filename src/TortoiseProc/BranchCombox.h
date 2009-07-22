#pragma once
#include "afxwin.h"
#include "LogDlg.h"
#include "BrowseRefsDlg.h"
#include "HistoryCombo.h"
#include "Balloon.h"
#include "HistoryCombo.h"
#include "AppUtils.h"

class CBranchCombox
{
public:
	CBranchCombox()
	{
		m_LocalBranchFilter = gPickRef_Head;
		m_RemoteBranchFilter = gPickRef_Remote;
		m_DialogName=_T("sync");
		m_pTooltip=NULL;
		
	}
protected:
	CHistoryCombo m_ctrlLocalBranch;
	CHistoryCombo m_ctrlRemoteBranch;
	int m_LocalBranchFilter;
	int m_RemoteBranchFilter;

	CBalloon *m_pTooltip;
	
	CString m_DialogName;
	
	CString m_RegKeyRemoteBranch;

	void  CbnSelchangeLocalBranch()
	{
		//Select pull-remote from current branch
		CString configName;
		CString currentBranch=this->m_ctrlLocalBranch.GetString();
		configName.Format(L"branch.%s.remote", currentBranch);
		CString pullRemote = g_Git.GetConfigValue(configName);

		this->SetRemote(pullRemote);

		//Select pull-branch from current branch
		configName.Format(L"branch.%s.merge", currentBranch);
		CString pullBranch = CGit::StripRefName(g_Git.GetConfigValue(configName));

		CString defaultUpstream;
		defaultUpstream.Format(L"remotes/%s/%s", pullRemote, pullBranch);
		int found = m_ctrlRemoteBranch.FindStringExact(0, defaultUpstream);
		if(found >= 0)
		{
			m_ctrlRemoteBranch.SetCurSel(found);
		}
		else if(!pullBranch.IsEmpty())
		{
			m_ctrlRemoteBranch.AddString(pullBranch);
		}
		else if(!this->m_RegKeyRemoteBranch.IsEmpty())
		{
			m_ctrlRemoteBranch.AddString(currentBranch);
		}
		
		this->AddBranchToolTips(&this->m_ctrlLocalBranch,this->m_pTooltip);

		LocalBranchChange();
	};
	void  CbnSelchangeRemoteBranch()
	{
		if(this->m_RegKeyRemoteBranch.IsEmpty())
			this->AddBranchToolTips(&this->m_ctrlRemoteBranch,this->m_pTooltip);

		RemoteBranchChange();
	}
	void  BnClickedButtonBrowseLocalBranch()
	{
		if(CBrowseRefsDlg::PickRefForCombo(&m_ctrlLocalBranch, m_LocalBranchFilter))	
			CbnSelchangeLocalBranch();
	}
	void  BnClickedButtonBrowseRemoteBranch()
	{
		if(!this->m_RegKeyRemoteBranch.IsEmpty())
		{
			CString remoteBranchName;
			CString remoteName;
			this->m_ctrlRemoteBranch.GetWindowText(remoteBranchName);
			//remoteName = m_Remote.GetString();
			//remoteBranchName = remoteName + '/' + remoteBranchName;
			remoteBranchName = CBrowseRefsDlg::PickRef(false, remoteBranchName, gPickRef_Remote);
			if(remoteBranchName.IsEmpty())
				return; //Canceled

			remoteBranchName = remoteBranchName.Mid(13);//Strip 'refs/remotes/'
			int slashPlace = remoteBranchName.Find('/');
			remoteName = remoteBranchName.Left(slashPlace);
			remoteBranchName = remoteBranchName.Mid(slashPlace + 1); //Strip remote name (for example 'origin/')

			//Select remote
			//int remoteSel = m_Remote.FindStringExact(0,remoteName);
			//if(remoteSel >= 0)
			//	m_Remote.SetCurSel(remoteSel);
			this->SetRemote(remoteName);

			//Select branch
			m_ctrlRemoteBranch.AddString(remoteBranchName);
			CbnSelchangeRemoteBranch();

		}else
		{
			if(CBrowseRefsDlg::PickRefForCombo(&m_ctrlRemoteBranch, m_RemoteBranchFilter))	
				CbnSelchangeRemoteBranch();
		}
	}

	virtual void LocalBranchChange(){};
	virtual void RemoteBranchChange(){};
	virtual void SetRemote(CString remote){};

	void AddBranchToolTips(CHistoryCombo *pBranch,CBalloon *tip)
	{
		if(pBranch&&tip)
		{
			CString text=pBranch->GetString();
			CString tooltip;
			BYTE_VECTOR data;
			g_Git.GetLog(data,text,NULL,1,0);
			GitRev rev;
			rev.ParserFromLog(data);
			tooltip.Format(_T("CommitHash:%s\nCommit by: %s  %s\n <b>%s</b> \n %s"),
				rev.m_CommitHash,
				rev.m_AuthorName,
				CAppUtils::FormatDateAndTime(rev.m_AuthorDate,DATE_LONGDATE),
				rev.m_Subject,
				rev.m_Body);

			pBranch->DisableTooltip();
			
			tip->AddTool(pBranch->GetComboBoxCtrl(),tooltip);
		}
	}
		
	void LoadBranchInfo()
	{
		m_ctrlLocalBranch.SetMaxHistoryItems(0x7FFFFFFF);
		m_ctrlRemoteBranch.SetMaxHistoryItems(0x7FFFFFFF);

		STRING_VECTOR list;
		list.clear();
		int current;
		g_Git.GetBranchList(list,&current,CGit::BRANCH_LOCAL);
		m_ctrlLocalBranch.AddString(list);

		CString currentBranch = g_Git.GetSymbolicRef();
		
		if(this->m_RegKeyRemoteBranch.IsEmpty())
		{
			list.clear();
			g_Git.GetBranchList(list,&current,CGit::BRANCH_REMOTE);
		
			m_ctrlRemoteBranch.AddString(list);

		}else
		{
			m_ctrlRemoteBranch.LoadHistory(m_RegKeyRemoteBranch,_T("sync"));
		}

		if(!this->m_strLocalBranch.IsEmpty())
		{
			m_ctrlLocalBranch.AddString(m_strLocalBranch);
			currentBranch = m_strLocalBranch;
		}
		else
		{
			m_ctrlLocalBranch.SetCurSel(current);
		}

		if(!m_strRemoteBranch.IsEmpty())
		{
			m_ctrlRemoteBranch.AddString(m_strRemoteBranch);
			m_ctrlRemoteBranch.SetCurSel(m_ctrlRemoteBranch.GetCount()-1);
		}
		else
		{
			CbnSelchangeLocalBranch();
		}

		this->LocalBranchChange();
		this->RemoteBranchChange();
	}
	
public:
	CString m_strLocalBranch;
	CString m_strRemoteBranch;
};

#define BRANCH_COMBOX_DDX \
	DDX_Control(pDX, IDC_COMBOBOXEX_LOCAL_BRANCH,		m_ctrlLocalBranch);      \
	DDX_Control(pDX, IDC_COMBOBOXEX_REMOTE_BRANCH,		m_ctrlRemoteBranch);     \

#define BRANCH_COMBOX_EVENT \
	ON_CBN_SELCHANGE(IDC_COMBOBOXEX_LOCAL_BRANCH,   OnCbnSelchangeLocalBranch)	\
	ON_CBN_SELCHANGE(IDC_COMBOBOXEX_REMOTE_BRANCH,  OnCbnSelchangeRemoteBranch)	\
	ON_BN_CLICKED(IDC_BUTTON_LOCAL_BRANCH,			OnBnClickedButtonBrowseLocalBranch) \
	ON_BN_CLICKED(IDC_BUTTON_REMOTE_BRANCH,			OnBnClickedButtonBrowseRemoteBranch) \

#define BRANCH_COMBOX_ADD_ANCHOR() \
	AddAnchor(IDC_COMBOBOXEX_LOCAL_BRANCH,TOP_LEFT);\
	AddAnchor(IDC_COMBOBOXEX_REMOTE_BRANCH,TOP_RIGHT);\
	AddAnchor(IDC_BUTTON_LOCAL_BRANCH,TOP_LEFT);\
	AddAnchor(IDC_BUTTON_REMOTE_BRANCH,TOP_RIGHT);\
	AddAnchor(IDC_STATIC_REMOTE_BRANCH,TOP_RIGHT);

#define BRANCH_COMBOX_EVENT_HANDLE() \
	afx_msg void OnCbnSelchangeLocalBranch(){CbnSelchangeLocalBranch();} \
	afx_msg void OnCbnSelchangeRemoteBranch(){CbnSelchangeRemoteBranch();}\
	afx_msg void OnBnClickedButtonBrowseLocalBranch(){BnClickedButtonBrowseLocalBranch();}\
	afx_msg void OnBnClickedButtonBrowseRemoteBranch(){BnClickedButtonBrowseRemoteBranch();}
