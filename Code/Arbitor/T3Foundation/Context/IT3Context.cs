/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using T3Foundation.Documents;

namespace T3Foundation.Context
{
    /// <summary>
    /// Context class for T3Applications.
    /// IT3Context should be used for defining what can and is being edited for the said document/data.
    /// IT3Context will only contain basic global information for the document. you should inherit from this class and add to your context.
    /// The Design is based off of a Workflow Driven Tool Design. (See remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po&t=926s)
    /// </summary> 
    public interface IT3Context
    {
        /// <summary>
        /// Is This context managing data that is from a higher level language? (e.g. c++ DLL from a game engine)
        /// </summary>
        /// <value></value>
        public bool ExternalHLManager {get; set;}
        /// <summary>
        /// Assembly for data from a dll. this can be null if there is no assembly needed.
        /// </summary>
        /// <value></value>
        public Assembly HLAssembly {get; set;}
        /// <summary>
        /// allows a class to subscribe to data, knowing the changes/updates to subscribed file.
        /// </summary>
        /// <typeparam name="IT3Document"></typeparam>
        /// <param name="subname"></param>
        public void SubscribeToData<IT3Document>(string subname);
    }
}