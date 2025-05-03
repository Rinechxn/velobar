// src/menuitemmodel.cpp
#include "menuitemmodel.hpp"

MenuItemModel::MenuItemModel(QObject* parent)
    : QObject(parent)
{
}

QVariantList MenuItemModel::items() const
{
    return m_mainItems;
}

void MenuItemModel::setItems(const QVariantList& allItems)
{
    m_mainItems.clear();

    // Filter and store only main menu items
    for (const QVariant& item : allItems) {
        QVariantMap itemMap = item.toMap();
        if (itemMap["level"].toInt() == 0 &&
            !itemMap["is_separator"].toBool() &&
            !itemMap["text"].toString().isEmpty()) {
            m_mainItems.append(item);
        }
    }

    emit dataChanged();
}
