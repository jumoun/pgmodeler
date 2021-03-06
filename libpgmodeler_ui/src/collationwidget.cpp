/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2013 - Raphael Araújo e Silva <rkhaotix@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "collationwidget.h"

CollationWidget::CollationWidget(QWidget *parent): BaseObjectWidget(parent, OBJ_COLLATION)
{
	QStringList loc_list, encodings;
	QFrame *frame=nullptr;

	Ui_CollationWidget::setupUi(this);

	frame=generateInformationFrame(trUtf8("The fields <strong><em>Collation</em></strong>, <strong><em>Locale</em></strong>, <strong><em>LC_COLLATE & LC_CTYPE</em></strong> are mutually exclusive, so you have to set only one of them in order to properly handle a collation."));
	collation_grid->addWidget(frame, collation_grid->count()+1, 0, 1, 0);
	frame->setParent(this);

	configureFormLayout(collation_grid, OBJ_COLLATION);

	//Configures the encoding combobox
	EncodingType::getTypes(encodings);
	encodings.push_front(trUtf8("Not defined"));
	encoding_cmb->addItems(encodings);

	//Configures the localizations combobox
	for(int i=QLocale::C; i <= QLocale::Chewa; i++)
	{
		for(int i1=QLocale::Afghanistan; i1 <= QLocale::Zimbabwe; i1++)
			loc_list.append(QLocale(static_cast<QLocale::Language>(i),static_cast<QLocale::Country>(i1)).name());
	}

	loc_list.removeDuplicates();
	loc_list.sort();
	loc_list.push_front(trUtf8("Not defined"));

	lccollate_cmb->addItems(loc_list);
	lcctype_cmb->addItems(loc_list);
	locale_cmb->addItems(loc_list);

	parent_form->setMinimumSize(520, 415);
	parent_form->setMaximumHeight(415);

	connect(collation_sel, SIGNAL(s_objectSelected(void)), this, SLOT(resetFields(void)));
	connect(collation_sel, SIGNAL(s_selectorCleared(void)), this, SLOT(resetFields(void)));
	connect(locale_cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(resetFields(void)));
	connect(lcctype_cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(resetFields(void)));
	connect(lccollate_cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(resetFields(void)));
	connect(parent_form->apply_ok_btn,SIGNAL(clicked(bool)), this, SLOT(applyConfiguration(void)));
}

void CollationWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Collation *collation)
{
	BaseObjectWidget::setAttributes(model, op_list, collation, schema);

	if(collation)
	{
		int idx=-1;

		idx=encoding_cmb->findText(~collation->getEncoding());
		encoding_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

		if(!collation_sel->getSelectedObject())
		{
			idx=locale_cmb->findText(collation->getLocale());
			locale_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

			if(locale_cmb->currentIndex()==0)
			{
				idx=lcctype_cmb->findText(collation->getLocalization(LC_CTYPE));
				lcctype_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

				idx=lccollate_cmb->findText(collation->getLocalization(LC_COLLATE));
				lccollate_cmb->setCurrentIndex(idx < 0 ? 0 : idx);
			}
		}
	}
}

void CollationWidget::hideEvent(QHideEvent *event)
{
	resetFields();
	BaseObjectWidget::hideEvent(event);
}

void CollationWidget::resetFields(void)
{
	//Block object's signals to evict an infinite call to this method
	collation_sel->blockSignals(true);
	locale_cmb->blockSignals(true);
	lccollate_cmb->blockSignals(true);
	lcctype_cmb->blockSignals(true);

	//If there is no sender reset all fields
	if(!sender())
	{
		collation_sel->clearSelector();
		locale_cmb->setCurrentIndex(0);
		lccollate_cmb->setCurrentIndex(0);
		lcctype_cmb->setCurrentIndex(0);
	}
	//Resetting the collation selector and locale combo
	else if((sender()==lccollate_cmb || sender()==lcctype_cmb) &&
		 (lccollate_cmb->currentIndex() > 0 || lcctype_cmb->currentIndex() > 0))
	{
		collation_sel->clearSelector();
		locale_cmb->setCurrentIndex(0);
	}
	//Resetting the lc_??? combos
	else if((sender()==collation_sel || sender()==locale_cmb) &&
					(collation_sel->getSelectedObject()!=nullptr || locale_cmb->currentIndex() > 0))
	{
		lccollate_cmb->setCurrentIndex(0);
		lcctype_cmb->setCurrentIndex(0);

		//Additionally resets the collation selector or locale combo depending on sender()
		if(sender()==collation_sel && collation_sel->getSelectedObject()!=nullptr)
			locale_cmb->setCurrentIndex(0);
		else
			collation_sel->clearSelector();
	}

	collation_sel->blockSignals(false);
	locale_cmb->blockSignals(false);
	lccollate_cmb->blockSignals(false);
	lcctype_cmb->blockSignals(false);
}


void CollationWidget::applyConfiguration(void)
{
	try
	{
		Collation *collation=nullptr;

		startConfiguration<Collation>();

		collation=dynamic_cast<Collation *>(this->object);
		BaseObjectWidget::applyConfiguration();

		if(encoding_cmb->currentIndex() > 0)
			collation->setEncoding(EncodingType(encoding_cmb->currentText()));

		if(locale_cmb->currentIndex() > 0)
			collation->setLocale(locale_cmb->currentText());

		if(lccollate_cmb->currentIndex() > 0)
			collation->setLocalization(LC_COLLATE, lccollate_cmb->currentText());

		if(lcctype_cmb->currentIndex() > 0)
			collation->setLocalization(LC_CTYPE, lcctype_cmb->currentText());

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}
